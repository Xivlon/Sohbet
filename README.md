# Sohbet
Turkish Centered Social Media Site

## Version 0.2.0-academic

A C++ academic social media backend with secure authentication and user management.

## Features

### Phase 1: Foundation & Authentication ✅
- **User Registration**: POST /api/users with validation and uniqueness checks
- **Secure Password Hashing**: Production-ready bcrypt implementation (12 rounds)
- **User Authentication**: POST /api/login with JWT token generation
- **User Retrieval**: GET /api/users/:username for public profiles
- **SQLite Integration**: RAII database wrapper with migrations
- **Academic Profile Fields**: username, email, university, department, enrollment_year, primary_language

### Security Features ✅
- **bcrypt Password Hashing**: Replaces insecure placeholder implementation
- **JWT Authentication**: HS256 signed tokens with expiration
- **Password Protection**: No password/hash exposure in API responses
- **Input Validation**: Username patterns, email format, password strength

### API Endpoints

#### User Registration
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
```

**Response (201 Created):**
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

#### User Login
```bash
POST /api/login
Content-Type: application/json

{
  "username": "ali_student",
  "password": "StrongPass123"
}
```

**Response (200 OK):**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": 1,
    "username": "ali_student",
    "email": "ali@example.edu",
    "university": "Istanbul Technical University",
    "department": "Computer Engineering", 
    "enrollment_year": 2023,
    "primary_language": "Turkish"
  }
}
```

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

**Previous security warning resolved**: Placeholder std::hash implementation has been completely replaced with secure bcrypt.

## Architecture

### Components
- **Models**: User model with academic profile fields
- **Database**: SQLite RAII wrapper with migrations
- **Repositories**: Data access layer with CRUD operations  
- **Security**: bcrypt password hashing and JWT token management
- **Server**: HTTP request handlers and endpoint routing

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

## Dependencies
- **SQLite3**: Database storage
- **bcrypt**: Secure password hashing (via FetchContent)
- **OpenSSL**: HMAC-SHA256 for JWT signatures

## Roadmap

### Phase 2: Course & Study Groups (Planned)
- Course model with enrollment management
- StudyGroup model with membership
- Academic networking features

### Phase 3: Content & Interaction (Planned)
- Post creation and interaction
- Q&A system for academic help
- Real-time messaging

### Phase 4: Advanced Features (Planned)
- Email verification
- Rate limiting and security improvements
- Advanced search and filtering
- Analytics and insights
