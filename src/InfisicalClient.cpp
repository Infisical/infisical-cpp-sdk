#include "libinfisical/InfisicalClient.h"

#include <iostream>
#include <cpr/cpr.h>

namespace Infisical
{

  InfisicalClient::InfisicalClient(Config &config) : _config(config), _httpClient(config.getUrl()), _authClient(config, &_httpClient), _secretsClient(&_httpClient)
  {

    auto authentication = config.getAuthentication();
    if (authentication._authStrategy == AuthStrategy::UNIVERSAL_AUTH)
    {
      auto response = _authClient.universalAuthLogin(
          authentication._clientId,
          authentication._clientSecret);
      _httpClient.setDefaultHeader("Authorization", "Bearer " + response.accessToken);
    }
    else
    {
      throw std::invalid_argument("Unsupported authentication strategy");
    }
  }

  // Destructor implementation (needed for unique_ptr with incomplete type)
  InfisicalClient::~InfisicalClient() = default;

}
