cmake_minimum_required(VERSION 3.15)
project(Sohbet VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    src/db/database.cpp
    src/repositories/user_repository.cpp
    src/utils/hash.cpp
    src/security/bcrypt_wrapper.cpp
    src/security/jwt.cpp
    src/server/server.cpp
    src/main.cpp
)

# Main executable
add_executable(sohbet ${SOURCES})
target_link_libraries(sohbet SQLite::SQLite3 bcrypt OpenSSL::SSL OpenSSL::Crypto)

# Tests
enable_testing()
add_executable(test_user tests/test_user.cpp src/models/user.cpp)
add_executable(test_user_repository tests/test_user_repository.cpp src/models/user.cpp src/db/database.cpp src/repositories/user_repository.cpp src/utils/hash.cpp src/security/bcrypt_wrapper.cpp)
target_link_libraries(test_user_repository SQLite::SQLite3 bcrypt)
add_executable(test_bcrypt tests/test_bcrypt.cpp src/utils/hash.cpp src/security/bcrypt_wrapper.cpp)
target_link_libraries(test_bcrypt bcrypt)
add_executable(test_authentication tests/test_authentication.cpp src/models/user.cpp src/db/database.cpp src/repositories/user_repository.cpp src/utils/hash.cpp src/security/bcrypt_wrapper.cpp src/security/jwt.cpp src/server/server.cpp)
target_link_libraries(test_authentication SQLite::SQLite3 bcrypt OpenSSL::SSL OpenSSL::Crypto)

add_test(NAME test_user COMMAND test_user)
add_test(NAME test_user_repository COMMAND test_user_repository)
add_test(NAME test_bcrypt COMMAND test_bcrypt)
add_test(NAME test_authentication COMMAND test_authentication)