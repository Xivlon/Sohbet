![Sohbet]([https://raw.githubusercontent.com/Xivlon/Sohbet/main/Sohbetlogo.png](https://raw.githubusercontent.com/Xivlon/Sohbet/main/Sohbetlogo.png?token=GHSAT0AAAAAADK67LXNGK2MASWEU65ODK5E2GVDHAA))

# Sohbet

A Turkish-centered social media platform with C++ backend and React frontend.

## Version 0.2.0-academic

This project demonstrates modern full-stack development with:
- **Backend**: Pure C++ with SQLite, bcrypt, JWT authentication (fully migrated from Node.js)
- **Frontend**: React with TypeScript, Material-UI components
- **Architecture**: RESTful API communication between React frontend and pure C++ backend

## Quick Start

### Prerequisites
- **C++ Environment**: cmake, make, SQLite3, OpenSSL (for backend)
- **Node.js**: Version 16+ with npm (for frontend only)

### Full Stack Startup
```bash
# Clone the repository
git clone https://github.com/Xivlon/Sohbet.git
cd Sohbet

# Run both backend and frontend
./start-fullstack.sh
```

This will:
1. Build the pure C++ backend (if needed)
2. Install React frontend dependencies (if needed)  
3. Start C++ backend server on port 8080
4. Start frontend development server on port 3000
5. Open your browser to http://localhost:3000

### Manual Setup

#### Backend Only
```bash
mkdir build && cd build
cmake ..
make
./sohbet
```

#### Frontend Only
```bash
cd frontend
npm install
npm start
```

---

## Backend Migration Status ✅

**Migration Complete**: The backend has been fully migrated from Node.js to pure C++17. This includes:

- ✅ **Complete Node.js Removal**: All Express.js, package.json, and Node.js backend dependencies eliminated
- ✅ **C++ HTTP Server**: Custom HTTP server implementation with socket programming
- ✅ **Database Layer**: SQLite integration with RAII wrappers and prepared statements
- ✅ **Authentication System**: bcrypt password hashing + JWT token generation
- ✅ **API Endpoints**: Full REST API (`/api/status`, `/api/users`, `/api/login`)
- ✅ **Build System**: CMake configuration with proper dependency management
- ✅ **Testing Suite**: Comprehensive unit and integration tests for C++ backend

The frontend remains React+TypeScript and communicates with the C++ backend via REST API.

---

## Features

### Backend (C++) ✅
- **User Registration**: `POST /api/users` with validation and uniqueness checks  
- **Secure Password Hashing**: Production-ready bcrypt implementation (12 rounds)  
- **User Authentication**: `POST /api/login` with JWT token generation  
- **User Retrieval**: `GET /api/users/:username` for public profiles  
- **SQLite Integration**: RAII database wrapper with migrations  
- **Academic Profile Fields**: `username`, `email`, `university`, `department`, `enrollment_year`, `primary_language`  

### Frontend (React) ✅
- **Modern UI**: Material-UI components with responsive design
- **User Authentication**: Login and registration forms with validation
- **User Dashboard**: Profile display with academic information
- **Real-time Integration**: Live communication with C++ backend API
- **Status Display**: Server health monitoring and demo user information
- **TypeScript**: Type-safe development with comprehensive interfaces

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

### Demo User

For testing purposes, a demo user is automatically created when the server starts:

**Demo Credentials:**
- Username: `demo_student`
- Password: `demo123`

**Demo Login Example:**
```bash
curl -X POST -H "Content-Type: application/json" \
  -d '{"username":"demo_student","password":"demo123"}' \
  http://localhost:8080/api/login
```

This returns a valid JWT token and user data for the demo account.

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

### Quick Start

For the fastest setup experience, use the setup script:

```bash
./setup.sh
```

This script will:
- Check prerequisites (CMake, Make, G++, curl)
- Build the C++ backend automatically
- Start the HTTP server on port 8080
- Test all API endpoints
- Provide troubleshooting information

### Manual Build Process

#### Prerequisites
- CMake 3.16+
- C++17 compiler (GCC/Clang)
- SQLite3 development libraries
- OpenSSL development libraries
- Make (build-essential)

#### Build Steps
```bash
mkdir build && cd build
cmake ..
make
cd ..
```

#### Run Server
```bash
cd build
./sohbet
```

#### Test Endpoints
```bash
curl http://localhost:8080/api/status
curl http://localhost:8080/api/users/demo
```

### Full Stack Development

For frontend + backend development:

```bash
./start-fullstack.sh
```

This starts both the C++ backend (port 8080) and React frontend (port 3000) with automatic proxy setup.

### Run Tests
```bash
cmake --build build --target test
```

## Troubleshooting

### Backend Server Issues

**Problem**: `ECONNREFUSED` error when frontend tries to connect

**Solutions**:
1. **Check if server is running**:
   ```bash
   curl http://localhost:8080/api/status
   # Should return: {"status":"ok","version":"0.2.0-academic",...}
   ```

2. **Check port availability**:
   ```bash
   lsof -i :8080
   # Should show sohbet process listening on port 8080
   ```

3. **Restart server**:
   ```bash
   # Kill existing server
   pkill -f sohbet
   
   # Start fresh
   ./setup.sh
   ```

**Problem**: Server fails to start

**Solutions**:
1. **Check build status**:
   ```bash
   ls -la build/sohbet
   # Should exist and be executable
   ```

2. **Review server logs**:
   ```bash
   tail -f build/server_output.log  # If using setup.sh
   # OR run manually to see output:
   cd build && ./sohbet
   ```

3. **Check dependencies**:
   ```bash
   ldd build/sohbet  # Check linked libraries
   ```

**Problem**: Permission denied

**Solutions**:
```bash
chmod +x build/sohbet
chmod +x setup.sh
```

**Problem**: Port 8080 already in use

**Solutions**:
```bash
# Find what's using port 8080
lsof -i :8080

# Kill the process (replace PID with actual process ID)
kill <PID>

# Or find and kill sohbet processes
pkill -f sohbet
```

### Build Issues

**Problem**: CMake configuration fails

**Solutions**:
1. **Install missing dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt update
   sudo apt install cmake build-essential libsqlite3-dev libssl-dev

   # macOS (with Homebrew)
   brew install cmake sqlite openssl
   ```

2. **Check CMake version**:
   ```bash
   cmake --version  # Should be 3.16+
   ```

3. **Clean build directory**:
   ```bash
   rm -rf build
   mkdir build && cd build
   cmake ..
   ```

**Problem**: Make build fails

**Solutions**:
1. **Check compiler version**:
   ```bash
   g++ --version  # Should support C++17
   ```

2. **Clean rebuild**:
   ```bash
   cd build
   make clean
   make
   ```

3. **Verbose build to see errors**:
   ```bash
   make VERBOSE=1
   ```

### Frontend Proxy Issues

**Problem**: Frontend can't reach backend API

**Verification**:
1. **Backend server running**: `curl http://localhost:8080/api/status`
2. **Frontend proxy configured**: Check `frontend/package.json` has `"proxy": "http://localhost:8080"`
3. **CORS headers**: Server should return `Access-Control-Allow-Origin: *`

**Solution**: Use the full stack startup script:
```bash
./start-fullstack.sh
```

### Testing & Verification

**Complete endpoint test**:
```bash
# GET endpoints
curl -v http://localhost:8080/api/status
curl -v http://localhost:8080/api/users/demo

# POST endpoint
curl -v -X POST -H "Content-Type: application/json" \
  -d '{"username":"test","email":"test@example.edu","password":"TestPass123"}' \
  http://localhost:8080/api/users
```

**Check server health**:
```bash
# Server should respond with status info
curl -s http://localhost:8080/api/status | python3 -m json.tool
```
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
