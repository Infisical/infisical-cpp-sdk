#include "libinfisical/InfisicalClient.h"

namespace Infisical
{

  /*
   * Authenticate with Infisical using Machine Identity Universal Auth
   * @params
   *   - `clientId`: The client ID of your Universal Auth Machine Identity
   *   - `clientSecret`: The client secret of your Universal Auth Machine Identity
   */
  Infisical::AuthenticationBuilder &AuthenticationBuilder::withUniversalAuth(std::string clientId, std::string clientSecret)
  {
    _authentication._authStrategy = AuthStrategy::UNIVERSAL_AUTH;
    _authentication._clientId = std::move(clientId);
    _authentication._clientSecret = std::move(clientSecret);
    return *this;
  }

  /*
   * Authenticate with Infisical using Machine Identity Universal Auth, using environment variables.
   *
   *  Set the `INFISICAL_MACHINE_IDENTITY_CLIENT_ID` environment variable to your Machine Identity Universal Auth Client ID
   *  Set the `INFISICAL_MACHINE_IDENTITY_CLIENT_SECRET` environment variable to your Machine Identity Universal Auth Client Secret
   */
  Infisical::AuthenticationBuilder &AuthenticationBuilder::withUniversalAuth()
  {
    auto clientId = std::getenv("INFISICAL_MACHINE_IDENTITY_CLIENT_ID");
    auto clientSecret = std::getenv("INFISICAL_MACHINE_IDENTITY_CLIENT_SECRET");

    _authentication._authStrategy = AuthStrategy::UNIVERSAL_AUTH;
    _authentication._clientId = std::move(clientId);
    _authentication._clientSecret = std::move(clientSecret);
    return *this;
  }

  Infisical::Authentication AuthenticationBuilder::build()
  {
    return std::move(_authentication);
  }
}
