cmake_minimum_required(VERSION 3.16)
project(Sohbet VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include FetchContent for external dependencies
include(FetchContent)

# Fetch bcrypt library
FetchContent_Declare(
  bcrypt
  GIT_REPOSITORY https://github.com/trusch/libbcrypt.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(bcrypt)

# Find packages
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(include)
include_directories(${CMAKE_BINARY_DIR}/_deps/bcrypt-src/include)

# Source files
set(SOURCES
    src/models/user.cpp
    src/models/role.cpp
    src/models/voice_channel.cpp
    src/db/database.cpp
    src/repositories/user_repository.cpp
    src/repositories/role_repository.cpp
    src/services/permission_service.cpp
    src/utils/hash.cpp
    src/security/bcrypt_wrapper.cpp
    src/security/jwt.cpp
    src/server/server.cpp
    src/voice/voice_config.cpp
    src/voice/voice_service.cpp
)

# Create library
add_library(sohbet_lib ${SOURCES})
target_link_libraries(sohbet_lib SQLite::SQLite3 bcrypt OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(sohbet_lib PUBLIC include)

# Main executable
add_executable(sohbet src/main.cpp)
target_link_libraries(sohbet sohbet_lib)

# Enable testing
enable_testing()

# Test executables
add_executable(test_user tests/test_user.cpp)
target_link_libraries(test_user sohbet_lib)
add_test(NAME UserTest COMMAND test_user)

add_executable(test_user_repository tests/test_user_repository.cpp)
target_link_libraries(test_user_repository sohbet_lib)
add_test(NAME UserRepositoryTest COMMAND test_user_repository)

add_executable(test_bcrypt tests/test_bcrypt.cpp)
target_link_libraries(test_bcrypt sohbet_lib bcrypt)
add_test(NAME BcryptTest COMMAND test_bcrypt)

add_executable(test_authentication tests/test_authentication.cpp)
target_link_libraries(test_authentication sohbet_lib)
add_test(NAME AuthenticationTest COMMAND test_authentication)

add_executable(test_voice_service tests/test_voice_service.cpp)
target_link_libraries(test_voice_service sohbet_lib)
add_test(NAME VoiceServiceTest COMMAND test_voice_service)

# Compiler flags
target_compile_options(sohbet_lib PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(sohbet PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_user PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_user_repository PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_bcrypt PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_authentication PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_voice_service PRIVATE -Wall -Wextra -Wpedantic)