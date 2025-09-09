# Sohbet
C++ Academic Social Media Backend

## Overview
Sohbet is an academic social media backend designed specifically for university students and educational communities. This C++ implementation provides a robust foundation for academic networking, course collaboration, and knowledge sharing.

## Version: 0.2.0-academic

### Features
- **User Registration & Authentication**: Secure user registration with academic profile fields
- **SQLite Database**: Lightweight persistence layer with RAII wrapper
- **Academic Profiles**: Support for university, department, enrollment year, and language preferences
- **RESTful API**: Clean HTTP endpoints for client integration

### Security Notice
⚠️ **WARNING**: The current implementation uses a placeholder hashing mechanism (std::hash + salt) for passwords. This is **INSECURE** and MUST be replaced with bcrypt or Argon2 before any production deployment.

## API Endpoints

### GET /api/status
Returns server status and version information.

**Response (200):**
```json
{
    "status": "ok",
    "version": "0.2.0-academic",
    "service": "Sohbet Academic Social Backend"
}
```

### GET /api/users/demo
Returns example usage for user registration.

**Response (200):**
```json
{
    "message": "User registration endpoint available at POST /api/users",
    "example": {
        "username": "ali_student",
        "email": "ali@example.edu",
        "password": "StrongPass123",
        "university": "Istanbul Technical University",
        "department": "Computer Engineering",
        "enrollment_year": 2022,
        "primary_language": "Turkish",
        "additional_languages": ["English", "German"]
    }
}
```

### POST /api/users
Create a new user account.

**Request Body:**
```json
{
    "username": "ali_student",
    "email": "ali@example.edu",
    "password": "StrongPass123",
    "university": "Istanbul Technical University",
    "department": "Computer Engineering",
    "enrollment_year": 2022,
    "primary_language": "Turkish",
    "additional_languages": ["English", "German"]
}
```

**Validation Rules:**
- `username`: 3-32 characters, alphanumeric and underscores only
- `email`: Must contain '@' symbol
- `password`: Minimum 8 characters
- Uniqueness enforced on username and email

**Success Response (201):**
```json
{
    "id": 1,
    "username": "ali_student",
    "email": "ali@example.edu",
    "university": "Istanbul Technical University",
    "department": "Computer Engineering",
    "enrollment_year": 2022,
    "primary_language": "Turkish",
    "additional_languages": ["English", "German"]
}
```

**Error Responses:**
- `400`: Validation error
- `409`: Username or email already exists
- `500`: Internal server error

## Building & Running

### Prerequisites
- CMake 3.16+
- C++17 compatible compiler
- SQLite3 development libraries

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

### Running Tests
```bash
ctest -V
```

### Running the Server
```bash
./sohbet
```

### Manual Testing with curl
```bash
# Register a new user
curl -X POST http://localhost:8080/api/users \
  -H 'Content-Type: application/json' \
  -d '{
    "username":"test_student",
    "email":"test@university.edu",
    "password":"SecurePass123",
    "university":"Example University",
    "department":"Computer Science",
    "enrollment_year":2023,
    "primary_language":"English"
  }'
```

## Database Schema

### Users Table
```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    university TEXT,
    department TEXT,
    enrollment_year INTEGER,
    primary_language TEXT,
    additional_languages TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

## Architecture

### Components
- **Models**: User data structures with JSON serialization
- **Database**: SQLite RAII wrapper for safe database operations
- **Repositories**: Data access layer with CRUD operations
- **Server**: HTTP request handling and routing
- **Utils**: Hashing utilities (placeholder implementation)

### Security Features
- Password hashing (placeholder - needs replacement)
- Input validation
- SQL injection prevention through prepared statements
- Password hashes never exposed in API responses

## Roadmap

### Phase 2 (Current) ✅
- User registration and authentication foundation
- SQLite database integration
- Basic API endpoints

### Phase 3 (Planned)
- Course and StudyGroup models
- Proper bcrypt/Argon2 password hashing
- JWT-based authentication
- Extended user profiles

### Phase 4 (Future)
- Posts and Q&A systems
- Real-time messaging
- File sharing capabilities
- Advanced search and filtering

## Contributing
This is an academic project focused on learning C++ backend development patterns and security practices.

## License
Educational use only - not intended for production deployment without security hardening.
