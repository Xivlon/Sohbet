cmake_minimum_required(VERSION 3.15)
project(Sohbet VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/models/user.cpp
    src/db/database.cpp
    src/repositories/user_repository.cpp
    src/utils/hash.cpp
    src/server/server.cpp
    src/main.cpp
)

# Main executable
add_executable(sohbet ${SOURCES})
target_link_libraries(sohbet SQLite::SQLite3)

# Tests
enable_testing()
add_executable(test_user tests/test_user.cpp src/models/user.cpp)
add_executable(test_user_repository tests/test_user_repository.cpp src/models/user.cpp src/db/database.cpp src/repositories/user_repository.cpp src/utils/hash.cpp)
target_link_libraries(test_user_repository SQLite::SQLite3)

add_test(NAME test_user COMMAND test_user)
add_test(NAME test_user_repository COMMAND test_user_repository)