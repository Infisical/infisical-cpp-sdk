#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

// we are using std::optional which nlohmann doesn't support by default, so we're overwriting the default serializer to support it
namespace nlohmann
{
  template <typename T>
  struct adl_serializer<std::optional<T>>
  {
    static void to_json(json &j, const std::optional<T> &opt)
    {
      if (opt.has_value())
      {
        j = *opt;
      }
      else
      {
        j = nullptr;
      }
    }

    static void from_json(const json &j, std::optional<T> &opt)
    {
      if (j.is_null())
      {
        opt = std::nullopt;
      }
      else
      {
        opt = j.get<T>();
      }
    }
  };
}

enum AuthStrategy
{
  UNIVERSAL_AUTH
};

namespace Infisical
{

  class InfisicalError : public std::runtime_error
  {
  public:
    /**
     * Constructor for InfisicalError
     * @param message Error message
     * @param statusCode HTTP status code
     * @param response Full response text
     */
    InfisicalError(const std::string &message, int statusCode, const std::string &response)
        : std::runtime_error(message),
          m_statusCode(statusCode),
          m_response(response) {}

    /**
     * Get the HTTP status code that caused this error
     * @return HTTP status code
     */
    int getStatusCode() const { return m_statusCode; }

    /**
     * Get the full response text
     * @return Response text
     */
    const std::string &getResponse() const { return m_response; }

  private:
    int m_statusCode;
    std::string m_response;
  };

  // forward refs
  class InfisicalClient;
  class Config;
  class AuthenticationBuilder;
  class Authentication;

  namespace Secrets
  {
    class SecretsClient;
  }

  namespace auth
  {
    class AuthClient;
  }

  namespace http
  {
    class HttpClient;
  }

  // --------------------- INPUTS

  namespace Input
  {

    class DeleteSecretOptions
    {
      std::string _projectId;
      std::string _environment;
      std::string _secretPath = "/";
      std::string _type = "shared";
      std::string _secretKey;

      friend class DeleteSecretOptionsBuilder;

    public:
      const std::string &getProjectId() const { return _projectId; }
      const std::string &getEnvironment() const { return _environment; }
      const std::string &getSecretPath() const { return _secretPath; }
      const std::string &getType() const { return _type; }
      const std::string &getSecretKey() const { return _secretKey; }
    };

    class CreateSecretOptions
    {
      std::string _projectId;
      std::string _environment;
      std::string _secretPath = "/";
      std::string _type = "shared";
      std::string _secretKey;
      std::string _secretComment;
      std::string _secretValue;
      std::vector<std::string> _tagIds;
      bool _skipMultilineEncoding;
      std::string _secretReminderNote;
      unsigned int _secretReminderRepeatDays;

      friend class CreateSecretOptionsBuilder;

    public:
      const std::string &getProjectId() const { return _projectId; }
      const std::string &getEnvironment() const { return _environment; }
      const std::string &getSecretComment() const { return _secretComment; }
      const std::string &getSecretPath() const { return _secretPath; }
      const std::string &getSecretKey() const { return _secretKey; }
      const std::string &getSecretValue() const { return _secretValue; }
      const std::vector<std::string> &getTagIds() const { return _tagIds; }
      bool getSkipMultilineEncoding() const { return _skipMultilineEncoding; }
      const std::string &getType() const { return _type; }
      const std::string &getSecretReminderNote() const { return _secretReminderNote; }
      const unsigned int &getSecretReminderRepeatDays() const { return _secretReminderRepeatDays; }
    };

    class UpdateSecretOptions
    {
      std::string _projectId;
      std::string _environment;
      std::string _secretPath = "/";
      std::string _type = "shared";
      std::string _secretKey;
      std::string _newSecretKey;
      std::string _secretValue;
      std::string _secretComment;
      std::string _secretReminderNote;
      unsigned int _secretReminderRepeatDays;
      std::vector<std::string> _tagIds;

      friend class UpdateSecretOptionsBuilder;

    public:
      const std::string &getProjectId() const { return _projectId; }
      const std::string &getEnvironment() const { return _environment; }
      const std::string &getSecretPath() const { return _secretPath; }
      const std::string &getSecretKey() const { return _secretKey; }
      const std::string &getNewSecretKey() const { return _newSecretKey; }
      const std::string &getSecretValue() const { return _secretValue; }
      const std::string &getSecretComment() const { return _secretComment; }
      const std::string &getSecretReminderNote() const { return _secretReminderNote; }
      const std::string &getType() const { return _type; }
      const unsigned int &getSecretReminderRepeatDays() const { return _secretReminderRepeatDays; }
      const std::vector<std::string> &getTagIds() const { return _tagIds; }
    };

    class GetSecretOptions
    {
      std::string _secretKey;
      std::string _projectId;
      std::string _environment;
      std::string _secretPath = "/";
      std::string _type = "shared";
      unsigned int _version;
      bool _expandSecretReferences = true;

      friend class GetSecretOptionsBuilder;

    public:
      const std::string &getProjectId() const { return _projectId; }
      const std::string &getEnvironment() const { return _environment; }
      const std::string &getSecretPath() const { return _secretPath; }
      const std::string &getSecretKey() const { return _secretKey; }
      unsigned int getVersion() const { return _version; }
      const std::string &getType() const { return _type; }
      bool getExpandSecretReferences() const { return _expandSecretReferences; }
    };

    class ListSecretOptions
    {
    private:
      std::string _projectId;
      std::string _environment;
      std::string _secretPath = "/";
      std::vector<std::string> _tagSlugs;
      bool _addSecretsToEnvironmentVariables;
      bool _recursive;
      bool _expandSecretReferences = true;

      friend class ListSecretOptionsBuilder;

    public:
      ListSecretOptions() = default;

      const std::string &getProjectId() const { return _projectId; }
      const std::string &getEnvironment() const { return _environment; }
      const std::string &getSecretPath() const { return _secretPath; }
      const std::vector<std::string> &getTagSlugs() const { return _tagSlugs; }
      bool getRecursive() const { return _recursive; }
      bool getAddSecretsToEnvironmentVariables() const { return _addSecretsToEnvironmentVariables; }
      bool getExpandSecretReferences() const { return _expandSecretReferences; }
    };

    class ListSecretOptionsBuilder
    {
      ListSecretOptions _options;

    public:
      ListSecretOptionsBuilder() = default;

      ListSecretOptions build()
      {
        if (_options._projectId.empty() || _options._environment.empty())
        {
          throw std::invalid_argument("ListSecretOptions: Project ID and Environment cannot be empty");
        }
        return _options;
      }

      ListSecretOptionsBuilder &withProjectId(const std::string &value)
      {
        _options._projectId = value;
        return *this;
      }

      ListSecretOptionsBuilder &withEnvironment(const std::string &value)
      {
        _options._environment = value;
        return *this;
      }

      ListSecretOptionsBuilder &withSecretPath(const std::string &value)
      {
        _options._secretPath = value;
        return *this;
      }

      ListSecretOptionsBuilder &withRecursive(bool value)
      {
        _options._recursive = value;
        return *this;
      }

      ListSecretOptionsBuilder &withAddSecretsToEnvironmentVariables(bool value)
      {
        _options._addSecretsToEnvironmentVariables = value;
        return *this;
      }

      ListSecretOptionsBuilder &withTagSlugs(const std::vector<std::string> &values)
      {
        _options._tagSlugs = values;
        return *this;
      }

      ListSecretOptionsBuilder &withExpandSecretReferences(bool value)
      {
        _options._expandSecretReferences = value;
        return *this;
      }
    };

    class GetSecretOptionsBuilder
    {
      GetSecretOptions _options;

    public:
      GetSecretOptionsBuilder() = default;

      GetSecretOptions build()
      {
        if (_options._projectId.empty() || _options._environment.empty())
        {
          throw std::invalid_argument("GetSecretOptions: Project ID and Environment cannot be empty");
        }
        if (_options._secretKey.empty())
        {
          throw std::invalid_argument("GetSecretOptions: Secret Key cannot be empty");
        }
        return _options;
      }

      GetSecretOptionsBuilder &withProjectId(const std::string &value)
      {
        _options._projectId = value;
        return *this;
      }

      GetSecretOptionsBuilder &withSecretKey(const std::string &value)
      {
        _options._secretKey = value;
        return *this;
      }

      GetSecretOptionsBuilder &withEnvironment(const std::string &value)
      {
        _options._environment = value;
        return *this;
      }

      GetSecretOptionsBuilder &withSecretPath(const std::string &value)
      {
        _options._secretPath = value;
        return *this;
      }

      GetSecretOptionsBuilder &withVersion(unsigned int value)
      {
        _options._version = value;
        return *this;
      }

      GetSecretOptionsBuilder &withType(const std::string &value)
      {
        _options._type = value;
        return *this;
      }

      GetSecretOptionsBuilder &withExpandSecretReferences(bool value)
      {
        _options._expandSecretReferences = value;
        return *this;
      }
    };

    class UpdateSecretOptionsBuilder
    {
      UpdateSecretOptions _options;

    public:
      UpdateSecretOptionsBuilder() = default;

      UpdateSecretOptions build()
      {
        if (_options._projectId.empty() || _options._environment.empty())
        {
          throw std::invalid_argument("UpdateSecretOptions: Project ID and Environment cannot be empty");
        }
        if (_options._secretKey.empty())
        {
          throw std::invalid_argument("UpdateSecretOptions: Secret Key cannot be empty");
        }
        return _options;
      }

      UpdateSecretOptionsBuilder &withProjectId(const std::string &value)
      {
        _options._projectId = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withType(const std::string &value)
      {
        _options._type = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withEnvironment(const std::string &value)
      {
        _options._environment = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withSecretPath(const std::string &value)
      {
        _options._secretPath = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withSecretKey(const std::string &value)
      {
        _options._secretKey = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withNewSecretKey(const std::string &value)
      {
        _options._newSecretKey = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withSecretValue(const std::string &value)
      {
        _options._secretValue = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withSecretComment(const std::string &value)
      {
        _options._secretComment = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withSecretReminderNote(const std::string &value)
      {
        _options._secretReminderNote = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withSecretReminderRepeatDays(const unsigned int &value)
      {
        _options._secretReminderRepeatDays = value;
        return *this;
      }

      UpdateSecretOptionsBuilder &withTagIds(const std::vector<std::string> &values)
      {
        _options._tagIds = values;
        return *this;
      }
    };

    class CreateSecretOptionsBuilder
    {
      CreateSecretOptions _options;

    public:
      CreateSecretOptionsBuilder() = default;

      CreateSecretOptions build()
      {
        if (_options._projectId.empty() || _options._environment.empty())
        {
          throw std::invalid_argument("CreateSecretOptions: Project ID and Environment cannot be empty");
        }
        if (_options._secretKey.empty())
        {
          throw std::invalid_argument("CreateSecretOptions: Secret Key cannot be empty");
        }
        return _options;
      }

      CreateSecretOptionsBuilder &withProjectId(const std::string &value)
      {
        _options._projectId = value;
        return *this;
      }

      CreateSecretOptionsBuilder &withSecretComment(const std::string &value)
      {
        _options._secretComment = value;
        return *this;
      }

      CreateSecretOptionsBuilder &withEnvironment(const std::string &value)
      {
        _options._environment = value;
        return *this;
      }

      CreateSecretOptionsBuilder &withSecretPath(const std::string &value)
      {
        _options._secretPath = value;
        return *this;
      }

      CreateSecretOptionsBuilder &withSecretKey(const std::string &value)
      {
        _options._secretKey = value;
        return *this;
      }

      CreateSecretOptionsBuilder &withSecretValue(const std::string &value)
      {
        _options._secretValue = value;
        return *this;
      }

      CreateSecretOptionsBuilder &withTagIds(const std::vector<std::string> &values)
      {
        _options._tagIds = values;
        return *this;
      }
    };

    class DeleteSecretOptionsBuilder
    {
      DeleteSecretOptions _options;

    public:
      DeleteSecretOptionsBuilder() = default;

      DeleteSecretOptions build()
      {
        if (_options._projectId.empty() || _options._environment.empty())
        {
          throw std::invalid_argument("DeleteSecretOptions: Project ID and Environment cannot be empty");
        }
        if (_options._secretKey.empty())
        {
          throw std::invalid_argument("DeleteSecretOptions: Secret Name cannot be empty");
        }
        return _options;
      }

      DeleteSecretOptionsBuilder &withProjectId(const std::string &value)
      {
        _options._projectId = value;
        return *this;
      }

      DeleteSecretOptionsBuilder &withEnvironment(const std::string &value)
      {
        _options._environment = value;
        return *this;
      }

      DeleteSecretOptionsBuilder &withSecretPath(const std::string &value)
      {
        _options._secretPath = value;
        return *this;
      }

      DeleteSecretOptionsBuilder &withType(const std::string &value)
      {
        _options._type = value;
        return *this;
      }

      DeleteSecretOptionsBuilder &withSecretKey(const std::string &value)
      {
        _options._secretKey = value;
        return *this;
      }
    };
  }

  namespace Secrets
  {
    class SecretMetadata
    {
      std::string key;
      std::string value;

    public:
      const std::string &getKey() const { return key; }
      const std::string &getValue() const { return value; }

      NLOHMANN_DEFINE_TYPE_INTRUSIVE(SecretMetadata, key, value)
    };

    class TSecret
    {
      std::string id;
      std::string workspace;
      std::string environment;
      unsigned int version;
      std::string type;
      std::string secretKey;
      std::string secretValue;
      std::string secretPath;
      bool skipMultilineEncoding;
      bool isRotatedSecret;
      std::optional<std::string> rotationId;
      std::vector<SecretMetadata> secretMetadata;

    public:
      const std::string &getId() const { return id; }
      const std::string &getWorkspace() const { return workspace; }
      const std::string &getEnvironment() const { return environment; }
      unsigned int getVersion() const { return version; }
      const std::string &getType() const { return type; }
      const std::string &getSecretKey() const { return secretKey; }
      const std::string &getSecretValue() const { return secretValue; }
      const std::string &getRotationId() const

      {
        static const std::string empty;
        return rotationId ? *rotationId : empty;
      }

      void _setSecretPath(const std::string &path)
      {
        secretPath = path;
      }

      const std::string &getSecretPath() const { return secretPath; }
      bool getSkipMultilineEncoding() const { return skipMultilineEncoding; }
      bool getIsRotatedSecret() const { return isRotatedSecret; }
      const std::vector<SecretMetadata> &getSecretMetadata() const { return secretMetadata; }

      // Custom from_json function to handle undefined paths like secretPath on imports, to avoid having a seperate class just for imported secrets...
      friend void from_json(const nlohmann::json &j, TSecret &secret)
      {
        // Required fields
        j.at("id").get_to(secret.id);
        j.at("workspace").get_to(secret.workspace);
        j.at("environment").get_to(secret.environment);
        j.at("version").get_to(secret.version);
        j.at("type").get_to(secret.type);
        j.at("secretKey").get_to(secret.secretKey);
        j.at("secretValue").get_to(secret.secretValue);
        j.at("skipMultilineEncoding").get_to(secret.skipMultilineEncoding);

        // Optional fields
        if (j.contains("secretPath"))
        {
          j.at("secretPath").get_to(secret.secretPath);
        }
        else
        {
          secret.secretPath = "";
        }

        if (j.contains("rotationId") && !j.at("rotationId").is_null())
        {
          secret.rotationId = j.at("rotationId").get<std::string>();
        }
        if (j.contains("isRotatedSecret"))
        {
          j.at("isRotatedSecret").get_to(secret.isRotatedSecret);
        }
        else
        {
          secret.rotationId = std::nullopt;
        }

        if (j.contains("secretMetadata"))
        {
          j.at("secretMetadata").get_to(secret.secretMetadata);
        }
        else
        {
          secret.secretMetadata.clear();
        }
      }

      // not used in reality, we're just adding this to make nlohmann::json happy so the macro works in other classes
      friend void to_json(nlohmann::json &j, const TSecret &secret)
      {
      }
    };

    class TImports
    {
      std::string secretPath;
      std::string environment;
      std::string folderId;
      std::vector<TSecret> secrets;

    public:
      const std::string &getSecretPath() const { return secretPath; }
      const std::string &getEnvironment() const { return environment; }
      const std::string &getFolderId() const { return folderId; }
      const std::vector<TSecret> &getSecrets() const { return secrets; }

      NLOHMANN_DEFINE_TYPE_INTRUSIVE(TImports, secretPath, environment, folderId, secrets)
    };

    class SecretsClient
    {
      http::HttpClient *httpClient;

    public:
      explicit SecretsClient(http::HttpClient *httpClient);

      std::vector<TSecret> listSecrets(Input::ListSecretOptions options);
      TSecret getSecret(Input::GetSecretOptions options);
      TSecret updateSecret(Input::UpdateSecretOptions options);
      TSecret createSecret(Input::CreateSecretOptions options);
      TSecret deleteSecret(Input::DeleteSecretOptions options);
    };
  }

  // -------------------------------------------- HTTP
  namespace http

  {

    struct TApiResponse
    {
      std::string message;
      std::string reqId;
    };

    /**
     * Enum representing HTTP methods
     */
    enum class Method
    {
      GET,
      POST,
      PATCH,
      DELETE
    };

    class HttpClient
    {
    public:
      HttpClient();
      explicit HttpClient(const std::string &baseUrl);

      void setBaseUrl(const std::string &baseUrl);
      void setDefaultHeader(const std::string &name, const std::string &value);
      cpr::Response request(
          Method method,
          const std::string &endpoint,
          const std::map<std::string, std::string> &headers = {},
          const std::map<std::string, std::string> &params = {},
          const std::string &body = "");

      cpr::Response get(
          const std::string &endpoint,
          const std::map<std::string, std::string> &headers = {},
          const std::map<std::string, std::string> &params = {});

      cpr::Response post(
          const std::string &endpoint,
          const std::map<std::string, std::string> &headers = {},
          const std::string &body = "");

      cpr::Response patch(
          const std::string &endpoint,
          const std::map<std::string, std::string> &headers = {},
          const std::string &body = "");

      cpr::Response del(
          const std::string &endpoint,
          const std::map<std::string, std::string> &headers = {},
          const std::string &body = "");

    private:
      std::string m_baseUrl;
      std::map<std::string, std::string> m_defaultHeaders;
      long m_timeout;

      cpr::Header mergeHeaders(const std::map<std::string, std::string> &headers);
      cpr::Parameters convertParams(const std::map<std::string, std::string> &params);
    };
  }

  // ------------------------ AUTH
  namespace auth
  {

    class Config;
    class HttpClient;

    class MachineIdentityLoginResponse
    {
    public:
      std::string accessToken;
      int expiresIn;
      int accessTokenMaxTTL;
      std::string tokenType;

      MachineIdentityLoginResponse() = default;
      MachineIdentityLoginResponse(
          const std::string &accessToken,
          int expiresIn,
          int accessTokenMaxTTL,
          const std::string &tokenType) : accessToken(accessToken),
                                          expiresIn(expiresIn),
                                          accessTokenMaxTTL(accessTokenMaxTTL),
                                          tokenType(tokenType) {}

      // macro to generate from_json and to_json functions
      NLOHMANN_DEFINE_TYPE_INTRUSIVE(MachineIdentityLoginResponse,
                                     accessToken, expiresIn,
                                     accessTokenMaxTTL, tokenType)
    };

    class AuthClient
    {
      friend class InfisicalClient;
      Infisical::Config &config;
      Infisical::http::HttpClient *httpClient;

    public:
      explicit AuthClient(Infisical::Config &config, http::HttpClient *httpClient);

      MachineIdentityLoginResponse universalAuthLogin(const std::string &clientId, const std::string &clientSecret);
      MachineIdentityLoginResponse universalAuthLogin();
    };
  }

  class Authentication
  {
  public:
    friend class AuthenticationBuilder;
    friend class InfisicalClient;

    AuthStrategy _authStrategy;
    std::string _clientId;
    std::string _clientSecret;
  };

  class AuthenticationBuilder
  {
    friend class InfisicalClient;

  public:
    explicit operator Authentication &&() { return std::move(_authentication); }

    AuthenticationBuilder &withUniversalAuth(std::string clientId, std::string clientSecret);
    AuthenticationBuilder &withUniversalAuth();
    Authentication build();

  private:
    Authentication _authentication;
  };

  // Config must be defined before ConfigBuilder since ConfigBuilder uses it
  class Config
  {
  public:
    friend class ConfigBuilder;
    friend class InfisicalClient;

    [[nodiscard]] std::string getUrl() const { return url_; }

    Authentication getAuthentication() const { return this->authentication_; }

  private:
    Config()
        : url_("") {}

    std::string url_;
    Authentication authentication_;
  };

  // Now define ConfigBuilder after Config is fully defined
  class ConfigBuilder
  {
  public:
    explicit operator Config &&() { return std::move(config_); }

    ConfigBuilder &withAuthentication(Authentication &&auth);
    ConfigBuilder &withHostUrl(std::string url);
    Config &build();

  private:
    Config config_;
  };

  class InfisicalClient
  {
    Infisical::Config _config;
    Infisical::http::HttpClient _httpClient;
    Infisical::auth::AuthClient _authClient;
    Infisical::Secrets::SecretsClient _secretsClient;

  public:
    explicit InfisicalClient(Config &config);
    ~InfisicalClient();
    Secrets::SecretsClient &secrets() { return _secretsClient; }
  };
} // namespace Infisical