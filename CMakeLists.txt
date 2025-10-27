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

add_executable(test_storage_service tests/test_storage_service.cpp)
target_link_libraries(test_storage_service sohbet_lib)
add_test(NAME StorageServiceTest COMMAND test_storage_service)

add_executable(test_multipart_parser tests/test_multipart_parser.cpp)
target_link_libraries(test_multipart_parser sohbet_lib)
add_test(NAME MultipartParserTest COMMAND test_multipart_parser)

add_executable(test_websocket_server tests/test_websocket_server.cpp)
target_link_libraries(test_websocket_server sohbet_lib)
add_test(NAME WebSocketServerTest COMMAND test_websocket_server)

# Compiler flags
target_compile_options(sohbet_lib PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(sohbet PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_user PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_user_repository PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_bcrypt PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_authentication PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_voice_service PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_storage_service PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_multipart_parser PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_websocket_server PRIVATE -Wall -Wextra -Wpedantic)