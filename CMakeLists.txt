cmake_minimum_required(VERSION 3.10)
project(libinfisical VERSION 0.1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG dd967cb48ea6bcbad9f1da5ada0db8ac0d532c06) # 1.11.2
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)


# Add include directories
include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

# Add library
add_library(infisical 
    src/InfisicalClient.cpp
    src/config/ConfigBuilder.cpp
    src/config/AuthenticationBuilder.cpp
    src/http/HttpClient.cpp
    src/auth/Auth.cpp
    src/secrets/SecretsClient.cpp

)

# Set public include directory for consumers of the library
target_include_directories(infisical PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# link against dependencies
target_link_libraries(infisical PUBLIC 
    cpr::cpr
    nlohmann_json::nlohmann_json
)

# Add example executable
add_executable(example examples/example.cpp)
target_link_libraries(example infisical)

# Installation rules
install(TARGETS infisical
    EXPORT infisical-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)