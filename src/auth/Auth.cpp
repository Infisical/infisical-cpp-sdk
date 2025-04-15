#include <libinfisical/InfisicalClient.h>

namespace Infisical
{

  namespace auth
  {

    Infisical::auth::AuthClient::AuthClient(Infisical::Config &config, Infisical::http::HttpClient *httpClient)
        : config(config), httpClient(httpClient)
    {
    }

    MachineIdentityLoginResponse AuthClient::universalAuthLogin(
        const std::string &clientId,
        const std::string &clientSecret)
    {
      nlohmann::json bodyJson = {
          {"clientId", clientId},
          {"clientSecret", clientSecret}};

      auto response = httpClient->post("/api/v1/auth/universal-auth/login", {}, bodyJson.dump());
      auto parsedResponse = nlohmann::json::parse(response.text).get<MachineIdentityLoginResponse>();

      httpClient->setDefaultHeader("Authorization", "Bearer " + parsedResponse.accessToken);

      return parsedResponse;
    }

  }
}