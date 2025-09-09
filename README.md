# Sohbet

A Turkish-centered social media backend written in C++.

## Version 0.2.0-academic
An academic-oriented backend featuring secure authentication and user management.

---

## Features

### Phase 1: Foundation & Authentication ✅
- **User Registration**: `POST /api/users` with validation and uniqueness checks  
- **Secure Password Hashing**: Production-ready bcrypt implementation (12 rounds)  
- **User Authentication**: `POST /api/login` with JWT token generation  
- **User Retrieval**: `GET /api/users/:username` for public profiles  
- **SQLite Integration**: RAII database wrapper with migrations  
- **Academic Profile Fields**: `username`, `email`, `university`, `department`, `enrollment_year`, `primary_language`  

### Security Features ✅
- **bcrypt Password Hashing**: Replaces insecure placeholder implementation  
- **JWT Authentication**: HS256 signed tokens with expiration  
- **Password Protection**: No password/hash exposure in API responses  
- **Input Validation**: Username patterns, email format, password strength  

---

## API Endpoints

### User Registration
```bash
POST /api/users
Content-Type: application/json

{
  "username": "ali_student",
  "email": "ali@example.edu", 
  "password": "StrongPass123",
  "university": "Istanbul Technical University",
  "department": "Computer Engineering",
  "enrollment_year": 2023,
  "primary_language": "Turkish"
}
###Response (201 Created):
{
  "id": 1,
  "username": "ali_student",
  "email": "ali@example.edu",
  "university": "Istanbul Technical University", 
  "department": "Computer Engineering",
  "enrollment_year": 2023,
  "primary_language": "Turkish"
}
###User Login
POST /api/login
Content-Type: application/json

{
  "username": "ali_student",
  "password": "StrongPass123"
}
###Response (200 OK):
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": 1,
    "username": "ali_student",
    "email": "ali@example.edu"
  }
}

**Roadmap**

Phase 2: Friendships and messaging

Phase 3: News feed and groups

Phase 4: Media uploads and notifications

##Build
```bash
mkdir build && cd build
cmake ..
make
./sohbet
```
###Run Tests
```bash
ctest
```
**Dev Notes**
##.gitignore
# Shared libraries
*.so
*.dylib
*.exe

# Database files
*.db
*.sqlite
!.gitkeep

# CMake generated files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
CTestTestfile.cmake
Testing/

# OS generated files
.DS_Store
.DS_Store?
._*
.Spotlight-V100
.Trashes
Thumbs.db

# Temporary files
*.tmp
*.bak
*~
.#*

# Log files
*.log

##CMakeLists.txt
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
    src/db/database.cpp
    src/repositories/user_repository.cpp
    src/utils/hash.cpp
    src/security/bcrypt_wrapper.cpp
    src/security/jwt.cpp
    src/server/server.cpp
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
target_link_libraries(test_bcrypt bcrypt)
add_test(NAME BcryptTest COMMAND test_bcrypt)

add_executable(test_authentication tests/test_authentication.cpp)
target_link_libraries(test_authentication sohbet_lib)
add_test(NAME AuthenticationTest COMMAND test_authentication)

# Compiler flags
target_compile_options(sohbet_lib PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(sohbet PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_user PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_user_repository PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_bcrypt PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(test_authentication PRIVATE -Wall -Wextra -Wpedantic)
#### User Retrieval
```bash
GET /api/users/:username
```

**Response (200 OK):**
```json
{
  "id": 1,
  "username": "ali_student",
  "email": "ali@example.edu",
  "university": "Istanbul Technical University",
  "department": "Computer Engineering",
  "enrollment_year": 2023,
  "primary_language": "Turkish"
}
```

#### Other Endpoints
- `GET /api/status` - Server status and version
- `GET /api/users/demo` - Demo user data

### Error Responses
- **400 Bad Request**: Invalid input format or validation errors
- **401 Unauthorized**: Invalid credentials
- **404 Not Found**: User not found
- **409 Conflict**: Username or email already exists
- **500 Internal Server Error**: Database or system errors

## Build & Run

### Prerequisites
- CMake 3.15+
- C++17 compiler
- SQLite3 development libraries
- OpenSSL development libraries

### Build
```bash
cmake -B build .
cmake --build build
```

### Run Tests
```bash
cmake --build build --target test
```

### Run Server
```bash
./build/sohbet
```

## Security Status ✅

- **Password Hashing**: Production-ready bcrypt with 12 rounds
- **Authentication**: JWT tokens with HMAC-SHA256 signatures
- **Data Protection**: No sensitive data in API responses
- **Input Validation**: Comprehensive validation and sanitization
### Security Notice
⚠️ **WARNING**: The current implementation uses a placeholder hashing mechanism (std::hash + salt) for password

**Previous security warning resolved**: Placeholder std::hash implementation has been completely replaced with secure bcrypt.
## Architecture

### Components
- **Models**: User model with academic profile fields and JSON serialization  
- **Database**: SQLite RAII wrapper with migrations and safe operations  
- **Repositories**: Data access layer with CRUD operations  
- **Security**: bcrypt password hashing and JWT token management  
- **Server**: HTTP request handlers and endpoint routing  
- **Utils**: Hashing utilities and input validation helpers  

### File Structure
```
include/
├── models/user.h
├── db/database.h
├── repositories/user_repository.h
├── security/bcrypt_wrapper.h
├── security/jwt.h
├── server/server.h
└── utils/hash.h

src/
├── models/user.cpp
├── db/database.cpp
├── repositories/user_repository.cpp
├── security/bcrypt_wrapper.cpp
├── security/jwt.cpp
├── server/server.cpp
├── utils/hash.cpp
└── main.cpp

tests/
├── test_user.cpp
├── test_user_repository.cpp  
├── test_bcrypt.cpp
└── test_authentication.cpp
```

---

## Dependencies
- **SQLite3**: Database storage  
- **bcrypt**: Secure password hashing (via FetchContent)  
- **OpenSSL**: HMAC-SHA256 for JWT signatures  

---

## Roadmap

### Phase 1: Foundation & Authentication (Complete)
- User Registration with validation and uniqueness checks  
- Secure Password Hashing (bcrypt, 12 rounds)  
- User Authentication with JWT token generation  
- SQLite integration with RAII database wrapper and migrations  
- Academic profile fields (username, email, university, department, enrollment_year, primary_language)  

### Phase 2: Course & Study Groups (Planned)
- Course model with enrollment management  
- StudyGroup model with membership  
- Academic networking features  

### Phase 3: Content & Interaction (Planned)
- Post creation and interaction  
- Q&A system for academic help  
- Real-time messaging  
- Extended user profiles  

### Phase 4: Advanced Features (Planned)
- Email verification  
- Rate limiting and security improvements  
- File sharing capabilities  
- Advanced search and filtering  
- Analytics and insights  


---

## Contributing
This is an academic project focused on learning C++ backend development patterns and security practices. Contributions are welcome, but please note this is not intended for production deployment without further hardening.

## License
Educational use only — not intended for production deployment without additional security auditing.