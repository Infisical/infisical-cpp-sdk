#include <libinfisical/InfisicalClient.h>
#include <iostream>
int main()
{

  auto clientId = std::getenv("INFISICAL_MACHINE_IDENTITY_CLIENT_ID");
  auto clientSecret = std::getenv("INFISICAL_MACHINE_IDENTITY_CLIENT_SECRET");
  auto projectId = std::getenv("INFISICAL_PROJECT_ID");
  auto environment = std::getenv("INFISICAL_ENVIRONMENT"); // dev, staging, prod, etc

  if (clientId == nullptr || clientSecret == nullptr || projectId == nullptr || environment == nullptr)
  {
    std::cout << "Please set the following environment variables: INFISICAL_MACHINE_IDENTITY_CLIENT_ID, INFISICAL_MACHINE_IDENTITY_CLIENT_SECRET, INFISICAL_PROJECT_ID, INFISICAL_ENVIRONMENT" << std::endl;
    return 1;
  }

  std::cout << "Starting app" << std::endl;

  Infisical::InfisicalClient client(
      Infisical::ConfigBuilder()
          .withHostUrl("https://app.infisical.com")
          .withAuthentication(
              Infisical::AuthenticationBuilder()
                  .withUniversalAuth(clientId, clientSecret)
                  .build())
          .build());

  const auto startKey = "TEST_KEY";
  const auto updateKey = "UPDATED_KEY";

  const auto createSecretOptions = Infisical::Input::CreateSecretOptionsBuilder()
                                       .withEnvironment(environment)
                                       .withProjectId(projectId)
                                       .withSecretKey(startKey)
                                       .withSecretValue("TEST_VALUE")
                                       .withSecretComment("Some comment")
                                       .build();

  const auto secret = client.secrets().createSecret(createSecretOptions);
  std::cout << "Created secret, key = " << secret.getSecretKey() << " value = " << secret.getSecretValue() << std::endl;

  const auto listSecretsOptions = Infisical::Input::ListSecretOptionsBuilder()
                                      .withProjectId(projectId)
                                      .withEnvironment(environment)
                                      .withSecretPath("/")
                                      .withRecursive(true)
                                      .withAddSecretsToEnvironmentVariables(true)
                                      .build();

  const auto secrets = client.secrets().listSecrets(listSecretsOptions);

  const auto res = std::getenv(startKey);

  if (res == nullptr || res != std::string("TEST_VALUE"))
  {
    std::cout << "Environment variable " << startKey << " not found or has wrong value" << std::endl;
    std::cout << "Expected value: TEST_VALUE, got: " << (res ? res : "null") << std::endl;
    return 1;
  }

  printf("The value of the environment variable %s is %s\n", startKey, res);

  if (secrets.size() != 1)
  {
    std::cout << "Expected 1 secret, got " << secrets.size() << std::endl;
    return 1;
  }

  if (secrets[0].getSecretKey() != startKey)
  {
    std::cout << "Expected secret key to be " << startKey << ", got " << secrets[0].getSecretKey() << std::endl;
    return 1;
  }

  const auto updateSecretOptions = Infisical::Input::UpdateSecretOptionsBuilder()
                                       .withEnvironment(environment)
                                       .withProjectId(projectId)
                                       .withSecretKey(startKey)
                                       .withNewSecretKey(updateKey)
                                       .build();

  const auto updatedSecret = client.secrets().updateSecret(updateSecretOptions);
  std::cout << "Updated secret, key = " << updatedSecret.getSecretKey() << " value = " << updatedSecret.getSecretValue() << std::endl;

  const auto getSecretOptions = Infisical::Input::GetSecretOptionsBuilder()
                                    .withEnvironment(environment)
                                    .withProjectId(projectId)
                                    .withSecretKey(updateKey)
                                    .build();

  const auto secret2 = client.secrets().getSecret(getSecretOptions);

  if (secret2.getSecretKey() != updateKey)
  {
    std::cout << "Expected secret key to be " << updateKey << ", got " << secret2.getSecretKey() << std::endl;
    return 1;
  }

  const auto deleteSecretOptions = Infisical::Input::DeleteSecretOptionsBuilder()
                                       .withEnvironment(environment)
                                       .withProjectId(projectId)
                                       .withSecretKey(updateKey)
                                       .build();

  const auto deletedSecret = client.secrets().deleteSecret(deleteSecretOptions);

  if (deletedSecret.getSecretKey() != updateKey)
  {
    std::cout << "Expected secret key to be " << updateKey << ", got " << deletedSecret.getSecretKey() << std::endl;
    return 1;
  }

  return 0;
}