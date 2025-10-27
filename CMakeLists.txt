cmake_minimum_required(VERSION 3.16)
project(Sohbet VERSION 0.2.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find packages
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Bcrypt source files (already downloaded)
set(BCRYPT_DIR ${CMAKE_BINARY_DIR}/_deps/bcrypt-src)
set(BCRYPT_SOURCES
    ${BCRYPT_DIR}/src/bcrypt.c
    ${BCRYPT_DIR}/src/crypt_blowfish.c
    ${BCRYPT_DIR}/src/crypt_gensalt.c
    ${BCRYPT_DIR}/src/wrapper.c
)

# Create bcrypt library from existing sources
add_library(bcrypt STATIC ${BCRYPT_SOURCES})
target_include_directories(bcrypt PUBLIC ${BCRYPT_DIR}/include)
target_include_directories(bcrypt PRIVATE ${BCRYPT_DIR}/include/bcrypt)

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
target_link_libraries(sohbet_lib SQLite::SQLite3 bcrypt OpenSSL::SSL OpenSSL::Crypto pthread)
target_include_directories(sohbet_lib PUBLIC include)

# Main executable
add_executable(sohbet src/main.cpp)
target_link_libraries(sohbet sohbet_lib)

# Compiler flags
target_compile_options(sohbet_lib PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(sohbet PRIVATE -Wall -Wextra -Wpedantic)
