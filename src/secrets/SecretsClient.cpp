#include <libinfisical/InfisicalClient.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include <cstdlib>
#ifdef _WIN32
#include <windows.h> // needed to set env vars on windows
#endif

std::string convertBooleanToString(bool v)
{
  return v ? "true" : "false";
}

bool setEnvironmentVariable(const std::string &key, const std::string &value)
{
  const char *current_value = std::getenv(key.c_str());
  if (current_value != nullptr)
  {
    return false;
  }

#ifdef _WIN32
  return SetEnvironmentVariableA(key.c_str(), value.c_str()) != 0;
#else // POSIX systems
  return setenv(key.c_str(), value.c_str(), 0) == 0;
#endif
}

void omitEmptyFieldsFromJson(nlohmann::json *j)
{
  if (j == nullptr)
  {
    return;
  }

  for (auto it = j->begin(); it != j->end();)
  {
    if (it.value().is_null() ||
        (it.value().is_string() && it.value().template get<std::string>().empty()) ||
        (it.value().is_array() && it.value().empty()))
    {
      it = j->erase(it);
    }
    else
    {
      ++it;
    }
  }
}

void omitEmptyFieldsFromMap(std::map<std::string, std::string> *params)
{
  if (params == nullptr)
  {
    return;
  }

  for (auto it = params->begin(); it != params->end();)
  {
    if (it->second.empty())
    {
      it = params->erase(it);
    }
    else
    {
      ++it;
    }
  }
}

void mergeSecretsAndImports(
    std::vector<Infisical::Secrets::TSecret> *secrets,
    std::vector<Infisical::Secrets::TImports> &imports)
{

  auto secretExists = [&secrets](const std::string &key)
  {
    return std::any_of(secrets->begin(), secrets->end(), [&key](const Infisical::Secrets::TSecret &secret)
                       { return secret.getSecretKey() == key; });
  };

  for (const auto &import : imports)
  {
    // we don't mind if this is a reference or not in this case, as we'll copy it over to the destination secrets, and we need to do a direct modify, C/C++(1086)
    for (auto importedSecret : import.getSecrets())
    {
      // if the imported secret doesn't exist in the destination already, push it in
      if (!secretExists(importedSecret.getSecretKey()))
      {
        importedSecret._setSecretPath(import.getSecretPath());
        secrets->push_back(importedSecret);
      }
    }
  }
}

void ensureUniqueSecretsByKey(std::vector<Infisical::Secrets::TSecret> *secrets)
{

  std::unordered_map<std::string, Infisical::Secrets::TSecret> secretMap;

  // use the loop to overwrite the entry with the last secret of the same key
  for (const auto &secret : *secrets)
  {
    secretMap[secret.getSecretKey()] = secret;
  }

  // clear the original secrets (this shouldn't fail. it couldn't. never ever. right?)
  secrets->clear();

  // refill the vector with only the unique secrets from the map
  for (const auto &pair : secretMap)
  {
    secrets->push_back(pair.second);
  }
}

namespace Infisical
{

  namespace Secrets
  {

    SecretsClient::SecretsClient(http::HttpClient *httpClient)
        : httpClient(httpClient)
    {
    }

    std::vector<TSecret> Secrets::SecretsClient::listSecrets(Infisical::Input::ListSecretOptions options)
    {
      auto params = std::map<std::string, std::string>{
          {"workspaceId", options.getProjectId()},
          {"environment", options.getEnvironment()},
          {"recursive", convertBooleanToString(options.getRecursive())},
          {"secretPath", options.getSecretPath()},
          {"include_imports", "true"},
          {"expandSecretReferences", convertBooleanToString(options.getExpandSecretReferences())}};
      omitEmptyFieldsFromMap(&params);

      if (options.getTagSlugs().size() > 0)
      {
        std::string tagSlugs = "";
        for (const auto &tagSlug : options.getTagSlugs())
        {
          if (!tagSlugs.empty())
          {
            tagSlugs += ",";
          }
          tagSlugs += tagSlug;
        }

        params["tagSlugs"] = tagSlugs;
      }

      auto parsedRaw = nlohmann::json::parse(this->httpClient->get("/api/v3/secrets/raw", {}, params).text);
      auto secrets = parsedRaw["secrets"].get<std::vector<TSecret>>();
      auto rawImports = parsedRaw["imports"];

      if (!rawImports.is_null())
      {
        auto imports = rawImports.get<std::vector<TImports>>();
        mergeSecretsAndImports(&secrets, imports);
      }

      if (options.getRecursive())
      {
        ensureUniqueSecretsByKey(&secrets);
      }

      if (options.getAddSecretsToEnvironmentVariables())
      {
        for (const auto &secret : secrets)
        {
          setEnvironmentVariable(secret.getSecretKey(), secret.getSecretValue());
        }
      }

      return secrets;
    }

    TSecret Secrets::SecretsClient::getSecret(Infisical::Input::GetSecretOptions options)
    {

      auto params = std::map<std::string, std::string>{
          {"workspaceId", options.getProjectId()},
          {"environment", options.getEnvironment()},
          {"secretPath", options.getSecretPath()},
          {"include_imports", "true"},
          {"type", options.getType()},
          {"expandSecretReferences", convertBooleanToString(options.getExpandSecretReferences())}};
      omitEmptyFieldsFromMap(&params);

      if (options.getVersion() > 0)
      {
        params["version"] = std::to_string(options.getVersion());
      }

      const auto url = "/api/v3/secrets/raw/" + options.getSecretKey();

      auto response = this->httpClient->get(url, {}, params).text;

      auto parsedRaw = nlohmann::json::parse(response);

      auto secret = parsedRaw["secret"].get<TSecret>();

      return secret;
    }

    TSecret Secrets::SecretsClient::updateSecret(Infisical::Input::UpdateSecretOptions options)
    {

      nlohmann::json bodyJson = {
          {"environment", options.getEnvironment()},
          {"workspaceId", options.getProjectId()},
          {"newSecretName", options.getNewSecretKey()},
          {"secretComment", options.getSecretComment()},
          {"secretPath", options.getSecretPath()},
          {"secretPath", options.getSecretPath()},
          {"type", options.getType()},
          {"secretReminderNote", options.getSecretReminderNote()},
          {"secretValue", options.getSecretValue()},
          {"tagIds", options.getTagIds()},
      };
      omitEmptyFieldsFromJson(&bodyJson);

      if (options.getSecretReminderRepeatDays() > 0)
      {
        bodyJson["secretReminderRepeatDays"] = options.getSecretReminderRepeatDays();
      }

      auto url = "/api/v3/secrets/raw/" + options.getSecretKey();

      auto response = this->httpClient->patch(url, {}, bodyJson.dump()).text;

      auto parsedRaw = nlohmann::json::parse(response);
      auto secret = parsedRaw["secret"].get<TSecret>();

      return secret;
    }

    TSecret Secrets::SecretsClient::createSecret(Infisical::Input::CreateSecretOptions options)
    {
      nlohmann::json bodyJson = {
          {"environment", options.getEnvironment()},
          {"workspaceId", options.getProjectId()},
          {"secretPath", options.getSecretPath()},
          {"secretComment", options.getSecretComment()},
          {"secretValue", options.getSecretValue()},
          {"secretReminderNote", options.getSecretReminderNote()},
          {"secretReminderRepeatDays", options.getSecretReminderRepeatDays()},
          {"tagIds", options.getTagIds()},
      };
      omitEmptyFieldsFromJson(&bodyJson);

      auto url = "/api/v3/secrets/raw/" + options.getSecretKey();
      auto parsedRaw = nlohmann::json::parse(this->httpClient->post(url, {}, bodyJson.dump()).text);
      auto secret = parsedRaw["secret"].get<TSecret>();

      return secret;
    }

    TSecret Secrets::SecretsClient::deleteSecret(Infisical::Input::DeleteSecretOptions options)
    {

      nlohmann::json bodyJson = {
          {"environment", options.getEnvironment()},
          {"workspaceId", options.getProjectId()},
          {"secretPath", options.getSecretPath()},
          {"type", options.getType()},
      };
      omitEmptyFieldsFromJson(&bodyJson);

      auto url = "/api/v3/secrets/raw/" + options.getSecretKey();

      auto parsedRaw = nlohmann::json::parse(this->httpClient->del(url, {}, bodyJson.dump()).text);

      auto secret = parsedRaw["secret"].get<TSecret>();

      return secret;
    }
  }

}