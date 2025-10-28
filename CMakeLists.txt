cmake_minimum_required(VERSION 3.16)
project(Sohbet VERSION 0.2.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find packages
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Download and build bcrypt library using FetchContent
include(FetchContent)
FetchContent_Declare(
    bcrypt
    GIT_REPOSITORY https://github.com/rg3/libbcrypt.git
    GIT_TAG master
)
FetchContent_MakeAvailable(bcrypt)

# Bcrypt source files from downloaded content
set(BCRYPT_DIR ${CMAKE_BINARY_DIR}/_deps/bcrypt-src)
set(BCRYPT_SOURCES
    ${BCRYPT_DIR}/bcrypt.c
    ${BCRYPT_DIR}/crypt_blowfish/crypt_blowfish.c
    ${BCRYPT_DIR}/crypt_blowfish/crypt_gensalt.c
    ${BCRYPT_DIR}/crypt_blowfish/wrapper.c
)

# Create bcrypt library from fetched sources
add_library(bcrypt_lib STATIC ${BCRYPT_SOURCES})
target_include_directories(bcrypt_lib PUBLIC ${BCRYPT_DIR})
target_include_directories(bcrypt_lib PUBLIC ${BCRYPT_DIR}/crypt_blowfish)

# Include directories
include_directories(include)
include_directories(${BCRYPT_DIR}/include)

# Source files
set(SOURCES
    src/models/user.cpp
    src/models/role.cpp
    src/models/media.cpp
    src/models/voice_channel.cpp
    src/models/friendship.cpp
    src/models/post.cpp
    src/models/comment.cpp
    src/models/group.cpp
    src/models/organization.cpp
    src/models/conversation.cpp
    src/models/message.cpp
    src/db/database.cpp
    src/db/migration_runner.cpp
    src/init/database_initializer.cpp
    src/helpers/user_helpers.cpp
    src/repositories/user_repository.cpp
    src/repositories/role_repository.cpp
    src/repositories/media_repository.cpp
    src/repositories/friendship_repository.cpp
    src/repositories/post_repository.cpp
    src/repositories/comment_repository.cpp
    src/repositories/group_repository.cpp
    src/repositories/organization_repository.cpp
    src/repositories/conversation_repository.cpp
    src/repositories/message_repository.cpp
    src/repositories/voice_channel_repository.cpp
    src/services/permission_service.cpp
    src/services/storage_service.cpp
    src/utils/hash.cpp
    src/utils/multipart_parser.cpp
    src/security/bcrypt_wrapper.cpp
    src/security/jwt.cpp
    src/server/server.cpp
    src/server/websocket_server.cpp
    src/voice/voice_config.cpp
    src/voice/voice_service.cpp
)

# Create library
add_library(sohbet_lib ${SOURCES})
target_link_libraries(sohbet_lib SQLite::SQLite3 bcrypt_lib OpenSSL::SSL OpenSSL::Crypto pthread)
target_include_directories(sohbet_lib PUBLIC include)

# Main executable
add_executable(sohbet src/main.cpp)
target_link_libraries(sohbet sohbet_lib)

# Compiler flags
target_compile_options(sohbet_lib PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(sohbet PRIVATE -Wall -Wextra -Wpedantic)

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
target_link_libraries(test_bcrypt sohbet_lib)
add_test(NAME BcryptTest COMMAND test_bcrypt)

add_executable(test_authentication tests/test_authentication.cpp)
target_link_libraries(test_authentication sohbet_lib)
add_test(NAME AuthenticationTest COMMAND test_authentication)

add_executable(test_voice_service tests/test_voice_service.cpp)
target_link_libraries(test_voice_service sohbet_lib)
add_test(NAME VoiceServiceTest COMMAND test_voice_service)

add_executable(test_storage_service tests/test_storage_service.cpp)
target_link_libraries(test_storage_service sohbet_lib)
add_test(NAME StorageServiceTest COMMAND test_storage_service)

add_executable(test_multipart_parser tests/test_multipart_parser.cpp)
target_link_libraries(test_multipart_parser sohbet_lib)
add_test(NAME MultipartParserTest COMMAND test_multipart_parser)

add_executable(test_websocket_server tests/test_websocket_server.cpp)
target_link_libraries(test_websocket_server sohbet_lib)
add_test(NAME WebSocketServerTest COMMAND test_websocket_server)

add_executable(test_config_env tests/test_config_env.cpp)
target_link_libraries(test_config_env sohbet_lib)
add_test(NAME ConfigEnvTest COMMAND test_config_env)

add_executable(test_user_helpers tests/test_user_helpers.cpp)
target_link_libraries(test_user_helpers sohbet_lib)
add_test(NAME UserHelpersTest COMMAND test_user_helpers)

add_executable(test_migration_runner tests/test_migration_runner.cpp)
target_link_libraries(test_migration_runner sohbet_lib)
add_test(NAME MigrationRunnerTest COMMAND test_migration_runner)

# Example programs
add_executable(user_management_example examples/user_management_example.cpp)
target_link_libraries(user_management_example sohbet_lib)
