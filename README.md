# Infisical C++ SDK

A C++ library implementation for [Infisical](https://infisical.com).

## Compatible with C++ 17 and later
The Infisical C++ SDK is compatible with C++ 17 capable compilers. This implies GCC 8 or newer, and clang 3.8 or newer. Earlier versions of C++ are unsupported.

## Dependencies
- `cURL`: Used internally for crafting HTTP requests.

## CMake Installation

```bash
cmake_minimum_required(VERSION 3.14)
project(InfisicalTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(OpenSSL REQUIRED)


include(FetchContent)


FetchContent_Declare(
  infisical
  GIT_REPOSITORY https://github.com/Infisical/infisical-cpp-sdk.git
  GIT_TAG 1.0.0 # Replace with the desired version
)

FetchContent_MakeAvailable(infisical)
FetchContent_GetProperties(infisical)


# Example usage. This will differ based on your project structure.
add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE infisical OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(my_app PRIVATE ${infisical_SOURCE_DIR}/include)
```

## Manual Installation
If you're unable to use the recommended CMake installation approach, you can choose to manually build the library and use it in your project.

```bash
mkdir build
cd build
cmake ..
make
```

## Quick-Start Example

Below you'll find an example that uses the Infisical SDK to fetch a secret with the key `API_KEY` using [Machine Identity Universal Auth](https://infisical.com/docs/documentation/platform/identities/universal-auth)

More examples can be found in the [/examples](https://github.com/Infisical/infisical-cpp-sdk/tree/main/examples) folder.

```cpp
#include <iostream>
#include <libinfisical/InfisicalClient.h>

int main() {

  try {
    Infisical::InfisicalClient client(
        Infisical::ConfigBuilder()
            .withHostUrl("https://app.infisical.com") // Optionally change this to your custom Infisical instance URL.
            .withAuthentication(
                Infisical::AuthenticationBuilder()
                    .withUniversalAuth("<machine-identity-universal-auth-client-id>", "<machine-identity-universal-auth-client-secret>")
                    .build())
            .build());

    const auto getSecretOptions = Infisical::Input::GetSecretOptionsBuilder()
                                      .withEnvironment("<env-slug>") // dev, staging, prod, etc
                                      .withProjectId("<your-project-id>")
                                      .withSecretKey("API_KEY")
                                      .build();

    const auto apiKeySecret = client.secrets().getSecret(getSecretOptions);

    printf("Secret retrieved, [key=%s] [value=%s]\n", secret.getSecretKey().c_str(), secret.getSecretValue().c_str());
  } catch (const Infisical::InfisicalError &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```

## JSON Serialization
The SDK uses [nlohmann/json](https://github.com/nlohmann/json) internally to serialize/deserialize JSON data. This SDK makes no assumptions about which JSON library you use in your project, and you aren't constrained to `nlohmann/json` in any way. Data returned by the SDK is returned as a class, which exposes Getter methods for getting fields such as the secret value or secret key.


## Documentation
The Infisical C++ SDK follows a builder pattern for all types of input. Below is a detailed documentation of our currently support methods.

Everything related to the Infisical SDK lives inside the `Infisical` namespace.

### InfisicalClient Class
`InfisicalClient(Config &config)`

```cpp
  Infisical::InfisicalClient client(
      Infisical::ConfigBuilder()
          .withHostUrl("https://app.infisical.com")
          .withAuthentication(
              Infisical::AuthenticationBuilder()
                  .withUniversalAuth(clientId, clientSecret)
                  .build())
          .build());
```

Config is created through the `ConfigBuilder` class. See below for more details

### Config Class

`Config` defines the configuration of the Infisical Client itself, such as authentication.

```cpp
Infisical::Config config = Infisical::ConfigBuilder()
                          .withHostUrl("https://app.infisical.com")
                          .withAuthentication(
                            Infisical::AuthenticationBuilder()
                              .withUniversalAuth(clientId, clientSecret)
                              .build())
                          .build();

Infisical::InfisicalClient client(config);
```

- `withHostUrl(string)` _(optional)_: Specify a custom Infisical host URL, pointing to your Infisical instance. Default sto `https://app.infisical.com`
- `withAuthentication(Infisical::Authentication)`: Configure the authentication that will be used by the SDK. See [Authentication Class](#authentication-class) for more details.
- `build()`: Returns the `Config` object with the options you configured.

### Authentication Class
```cpp
Infisical::Authentication auth = Infisical::AuthenticationBuilder()
                                  .withUniversalAuth(clientId, clientSecret)
                                  .build();

Infisical::Config config = Infisical::ConfigBuilder()
                            .withAuthentication(std::move(auth)) // Or use inline declaration
                            .build();
```

- `withUniversalAuth(string, string)`: Specify the Universal Auth Client ID and Client Secret that will be used for authentication.
- `build()`: Returns the `Authentication` object with the options you specified. 

### TSecret Class
The `TSecret` class is the class that's returned by all secret methods (get/list/delete/update/create). It can come in the form of a `std::vector` or a single instance. 

**Available getter methods:**
- `getId(): std::string`: Returns the ID of the secret.
- `getWorkspace(): std::string`: Returns the project ID of the secret.
- `getEnvironment(): std::string`: Returns the environment slug of the secret.
- `getVersion(): unsigned int`: Gets the version of the secret. By default this will always be the latest version unless specified otherwise with `withVersion()`
- `getType(): std::string`: Returns the type of the secret. Can only be `shared` or `personal`. Shared secrets are available to everyone with access to the secret. Personal secrets are personal overwrites of the secret, mainly intended for local development purposes.
- `getSecretKey(): std::string`: Returns the secret key.
- `getSecretValue(): std::string` Returns the secret value.
- `getRotationId(): std::string`: If the secret is a rotation secret, this will return the rotation ID of the secret. If it's a regular secret, this will return an empty string.
- `getSecretPath(): std::string`: Returns the secret path of the secret.
- `getSkipMultilineEncoding(): bool`: Returns wether or not skip multiline encoding is enabled for the secret or not.
`getIsRotatedSecret(): bool`: Returns wether or not the secret is a rotated secret. If `true`, then `getRotationId()` returns the ID of the rotation.



### Secrets

#### Create Secret
```cpp
const auto createSecretOptions = Infisical::Input::CreateSecretOptionsBuilder()
                                  .withEnvironment("<env-slug>")
                                  .withProjectId("<project-id>")
                                  .withSecretKey("SECRET_KEY_TO_CREATE")
                                  .withSecretValue("VALUE_TO_CREATE")
                                  .withSecretComment("Secret comment to attach") // Optional
                                  .withSecretPath("/path/where/to/create/secret") // Optional, defaults to /
                                  .withTagIds({"tag-id-1", "tag-id-2"}) // Optional
                                  .build();

const auto secret = client.secrets().createSecret(createSecretOptions);
```

**Parameters**:
- `withEnvironment(string)`: Specify the slug of the environment to create the secret in.
- `withProjectId(string)`: Specify the ID of the project to create the secret in.
- `withSecretPath(string)`: Specify the secret path to create the secret in. Defaults to `/`
- `withSecretKey(string)`: The secret key to be created.
- `withSecretValue(string)`: The value of the secret to create.
- `withSecretComment(string)` _(optional)_: Optionally add a comment to the secret.
- `withTagIds(std::vector<std::string>>)` _(optional)_: A list of ID's of tags to attach to the secret.
- `build()`: Returns the `CreateSecretOptions` class that can be passed into the `createSecret()` method.

**Returns**: 
- Returns the created secret as a `TSecret` class. Read more in the [TSecret Class](#tsecret-class) documentation.

#### Update Secret

```cpp
  const auto updateSecretOptions = Infisical::Input::UpdateSecretOptionsBuilder()
                                    .withEnvironment("<env-slug>")
                                    .withProjectId("<project-id>")
                                    .withSecretKey("<secret-key>") 
                                    .withNewSecretKey("<new-secret-key>") // Optional
                                    .withSecretValue("<new-secret-value>") // Optional
                                    .withSecretComment("Updated comment") // Optional
                                    .withSecretReminderNote("Updated reminder note") // Optional
                                    .withSecretReminderRepeatDays(1) // Optional
                                    .withType("shared") // Optional
                                    .withTagIds({"tag-id-3", "tag-id-4"}) // Optional
                                    .build();

const auto updatedSecret = client.secrets().updateSecret(updateSecretOptions);
```

**Parameters**:
- `withEnvironment(string)`: Specify the slug of the environment where the secret lives in.
- `withProjectId(string)`: Specify the ID of the project where the secret to update lives in.
- `withSecretPath(string)`: Specify the secret path of the secret to update. Defaults to `/`.
- `withType("shared" | "personal")`: _(optional)_: The type of secret to update. Defaults to `shared`.
- `withSecretKey(string)`: The key of the secret you wish to update.
- `withNewSecretKey(string)` _(optional)_: The new key of the secret you wish to update.
- `withSecretValue(string)` _(optional)_: The new value of the secret.
- `withSecretReminderNote(string)` _(optional)_: Update the secret reminder note attached to the secret.
- `withSecretReminderRepeatDays(unsigned int)` _(optional)_: Update the secret reminder repeat days attached to the secret.
- `withTagIds(std::vector<std::string>>)` _(optional)_: A list of ID's of tags to attach to the secret.
- `build()`: Returns the `UpdateSecretOptions` class that can be passed into the `updateSecret()` method.

**Returns**:
- Returns the updated secret as a `TSecret` class. Read more in the [TSecret Class](#tsecret-class) documentation.

#### Get Secret
```cpp
const auto getSecretOptions = Infisical::Input::GetSecretOptionsBuilder()
                                .withEnvironment("<env-slug>")
                                .withProjectId("<project-id>")
                                .withSecretKey("<secret-key>") 
                                .withType("shared")
                                .withVersion(2).
                                .withExpandSecretReferences(true)    
                                .build();

const auto secret = client.secrets().getSecret(getSecretOptions);
```
**Parameters**:
- `withEnvironment(string)`: Specify the slug of the environment where the secret lives in.
- `withProjectId(string)`: Specify the ID of the project where the secret lives in.
- `withSecretPath(string)`: Specify the secret path of the secret to get. Defaults to `/`
- `withType("shared" | "personal")`: _(optional)_: The type of secret to get. Defaults to `shared`.
- `withSecretKey(string)`: The key of the secret to get.
- `withExpandSecretReferences(bool)` _(optional)_: Wether or not to expand secret references automatically. Defaults to `true`.
- `withVersion(unsigned int)` _(optional)_: Optionally fetch a specific version of the secret. If not defined, the latest version of the secret is returned.
- `build()`: Returns the `GetSecretOptions` class that can be passed into the `getSecret()` method.

**Returns**:
- Returns the secret as a `TSecret` class. Read more in the [TSecret Class](#tsecret-class) documentation.

#### Delete Secret

```cpp
const auto deleteSecretOptions = Infisical::Input::DeleteSecretOptionsBuilder()
                                  .withEnvironment("<env-slug>")
                                  .withProjectId("<project-id>")
                                  .withSecretKey("<secret-key>")
                                  .withType("shared")
                                  .withSecretPath("<secret-path>")
                                  .build();

const auto deletedSecret = client.secrets().deleteSecret(deleteSecretOptions);
```

**Parameters**:
- `withEnvironment(string)`: Specify the slug of the environment where the secret to delete lives in.
- `withProjectId(string)`: Specify the ID of the project where the secret to delete lives in.
- `withSecretPath(string)`: Specify the secret path of the secret to delete. Defaults to `/`
- `withType("shared" | "personal")`: _(optional)_: The type of secret to delete. Defaults to `shared`.
- `withSecretKey(string)`: The key of the secret to delete.
- `build()` Returns the `DeleteSecretOptions` class that can be passed into the `deleteSecret()` method.

**Returns**:
- Returns the deleted secret as a `TSecret` class. Read more in the [TSecret Class](#tsecret-class) documentation.


#### List Secrets
```cpp
const auto listSecretsOptions = Infisical::Input::ListSecretOptionsBuilder()
                                  .withProjectId(projectId)
                                  .withEnvironment(environment)
                                  .withSecretPath("/")
                                  .withRecursive(false)
                                  .withAddSecretsToEnvironmentVariables(false)
                                  .build();

const auto secrets = client.secrets().listSecrets(listSecretsOptions);

```

**Parameters**:
- `withEnvironment(string)`: Specify the slug of the environment to list secrets from.
- `withProjectId(string)`: Specify the ID of the project to fetch secrets from.
- `withSecretPath(string)`: Specify the secret path to fetch secrets from. Defaults to `/`
- `withExpandSecretReferences(bool)` _(optional)_: Wether or not to expand secret references automatically. Defaults to `true`.
- `withRecursive(bool)` _(optional)_: Wether or not to recursively fetch secrets from sub-folders. If set to true, all secrets from the secret path specified with `withSecretPath()` and downwards will be fetched.
- `withAddSecretsToEnvironmentVariables(bool)` _(optional)_: If set to true, the fetched secrets will be automatically set as environment variables, making them accessible with `std::getenv` or equivalent by secret key.
- `build()`: Returns the `ListSecretsOptions` class that can be passed into the `listSecrets()` method.

**Returns**:
- Returns the listed secrets as `std::vector<TSecret>`. Read more in the [TSecret Class](#tsecret-class) documentation.

