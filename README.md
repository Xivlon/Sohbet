![Sohbet](https://raw.githubusercontent.com/Xivlon/Sohbet/main/Sohbetlogo.png)

# Sohbet

**Version 0.3.0-academic**

## What is Sohbet?

Sohbet is a **Turkish-centered academic social media platform** designed to help students connect, collaborate, and learn together. Whether you're looking for study partners, want to join academic discussions, or need help with coursework, Sohbet provides a safe and focused environment for academic networking.

### Key Features for Users

- 🎓 **Academic Profiles** - Connect with students from your university and department
- 🔒 **Secure & Private** - Your data is protected with industry-standard encryption
- 👥 **Friend System** - Send and accept friend requests to build your academic network
- 📝 **Social Posts** - Share thoughts and academic content with friends or publicly
- 💬 **Real-Time Chat** - Instant messaging with friends using WebSocket technology
- 📚 **Study Groups** - Professors can create groups for collaborative learning
- 🏛️ **Organizations** - Join clubs, societies, and academic organizations
- 🎙️ **Voice Channels** - Create and join voice discussion channels (Khave)
- 🌐 **Multilingual** - Built with Turkish and international students in mind
- 📱 **Modern Interface** - Clean, responsive design that works on all devices

### For Developers

This project showcases modern full-stack development:
- **Backend**: Pure C++ with SQLite database, bcrypt password hashing, and JWT authentication
- **Frontend**: React with TypeScript and Material-UI components
- **Architecture**: RESTful API for seamless communication between frontend and backend

---

## Getting Started

### Quick Start (Easiest Way)

The fastest way to get Sohbet running on your computer:

```bash
# 1. Download Sohbet
git clone https://github.com/Xivlon/Sohbet.git
cd Sohbet

# 2. Run the application
./start-fullstack.sh
```

This simple command will:
1. Set up everything automatically
2. Start the application
3. Open Sohbet in your web browser at http://localhost:3000

You can then create an account or use the demo login:
- **Username**: `demo_student`
- **Password**: `demo123`

### What You Need (Prerequisites)

Before running Sohbet, make sure you have:
- **For the backend** (server): cmake, make, SQLite3, OpenSSL
- **For the frontend** (user interface): Node.js version 16 or higher with npm

### Alternative Setup Options

If you prefer to run components separately:

#### Run Backend Server Only
```bash
mkdir build && cd build
cmake ..
make
./sohbet
```

#### Run Frontend Only
```bash
cd frontend
npm install
npm start
```

---

## Current Features

### What You Can Do Now

#### User Accounts & Profiles
- **Create an Account**: Register with your academic information (university, department, year)
- **Secure Login**: Your password is protected with industry-standard bcrypt encryption
- **Personal Profile**: Display your academic background and interests
- **Profile Photos**: Upload and display avatar images
- **Demo Account**: Try out the platform with our pre-configured demo account

#### Social Features
- **Friend Requests**: Send, accept, or reject friend requests
- **Friends List**: View and manage your academic connections
- **Posts & Feed**: Create posts with visibility controls (public, friends-only, or private)
- **Reactions**: React to posts with like, love, or insightful reactions
- **Comments & Replies**: Engage in threaded discussions on posts
- **Global Feed**: Discover public posts from the academic community

#### Groups & Organizations
- **Study Groups**: Professors can create groups for courses or study topics
- **Group Membership**: Join groups and collaborate with members
- **Organizations**: Explore and join academic clubs, societies, and departments
- **Organization Accounts**: Organizations can post announcements and updates

#### Real-Time Communication
- **Instant Messaging**: Real-time chat with friends using WebSocket technology
- **Typing Indicators**: See when someone is typing in a conversation
- **Online Presence**: Know who's currently online
- **Message History**: Access full conversation history with pagination
- **Voice Channels**: Create and join voice discussion rooms (API ready, UI in progress)

#### Security & Privacy
- ✅ **Strong Password Protection**: Passwords are hashed with bcrypt (12 computational rounds)
- ✅ **Secure Sessions**: JWT token authentication keeps you logged in safely
- ✅ **Data Protection**: Your sensitive information never appears in API responses
- ✅ **Input Validation**: All data is checked to prevent malicious inputs
- ✅ **Role-Based Access Control**: Permissions system for Students, Professors, and Admins

### Technical Features (For Developers)

#### Backend (C++)
- User registration with validation and uniqueness checks
- Secure password hashing with bcrypt
- JWT token generation for authentication
- User profile retrieval and management
- SQLite database with RAII wrapper and migrations
- Academic profile fields: username, email, university, department, enrollment year, primary language
- **Friend request system with status management**
- **Post creation with visibility controls (public/friends/private)**
- **Reaction system (like, love, insightful)**
- **Threaded comments and replies**
- **Group management (professor-created study groups)**
- **Organization/club management**
- **Conversation and message repositories**
- **WebSocket server for real-time communication (port 8081)**
- **Real-time chat message delivery**
- **Typing indicators and online presence tracking**
- **Voice channel management API**
- **Role-based permission system**
- **Media upload and storage service**

#### Frontend (React/Next.js/TypeScript)
- Modern UI with Shadcn/UI components
- Responsive design that works on all devices
- Login and registration forms with validation
- User dashboard with profile display
- **Friend requests and management interface**
- **Post composer with visibility controls**
- **Post feed with infinite scroll**
- **Comment threads with nested replies**
- **Reaction buttons and counters**
- **Group creation and browsing**
- **Organization directory**
- **Real-time chat interface**
- **WebSocket client with auto-reconnection**
- **React hooks for real-time features**
- **Typing indicators in chat**
- **Online/offline status indicators**
- **Avatar upload and display**
- **Permission gates for role-based features**
- Server health monitoring
- TypeScript for type-safe development

---

## API Reference (For Developers)

This section is for developers who want to understand how to interact with the Sohbet backend.

### User Registration

Create a new user account:

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

**Response** (201 Created):
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

### User Login

Authenticate and receive a login token:

```bash
POST /api/login
Content-Type: application/json

{
  "username": "ali_student",
  "password": "StrongPass123"
}
```

**Response** (200 OK):
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": 1,
    "username": "ali_student",
    "email": "ali@example.edu"
  }
}
```

### User Retrieval
This returns a valid JWT token and user data for the demo account.

### Authentication Flow

**Backend Flow**
1. **Registration** (`POST /api/users`):
   - Client sends user data with password over HTTPS (plain-text in transit, encrypted by TLS)
   - Server validates input (username pattern, email format, password strength)
   - Password is hashed using bcrypt (12 rounds)
   - User record stored in SQLite database
   - Server responds with user data (password hash excluded)

2. **Login** (`POST /api/login`):
   - Client sends username and password over HTTPS
   - Server looks up user by username
   - Password verified using bcrypt comparison
   - JWT token generated using HS256 (HMAC-SHA256 with shared secret)
   - Token payload includes: username, user_id, expiration (24h)
   - Server responds with `{ "token": "...", "user": { ... } }`

**Frontend Flow** (React + TypeScript)
1. User enters credentials in `LoginForm.tsx`
2. `ApiService.login()` calls backend `/api/login` endpoint
3. On success, JWT token stored and set in axios headers
4. Token included in `Authorization: Bearer <token>` header for authenticated requests
5. User data displayed in dashboard components

**Security Practices**
- Password hashes never returned in API responses (verified by tests)
- Input validation enforced on both frontend and backend
- JWT tokens signed with HS256 using a shared secret (server-side only)
- Tokens include expiration to limit validity window
- **Production Note**: Use HTTPS to encrypt password transmission; current implementation assumes secure transport layer

**Roadmap**

Phase 2: Friendships and messaging

Phase 3: News feed and groups

Get user profile information:

```bash
GET /api/users/:username
```

**Response** (200 OK):
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

### Other Endpoints

- `GET /api/status` - Server status and version information
- `GET /api/users/demo` - Demo user data for testing

### Demo User

A demo account is automatically created for testing:

- **Username**: `demo_student`
- **Password**: `demo123`

**Example:**
```bash
curl -X POST -H "Content-Type: application/json" \
  -d '{"username":"demo_student","password":"demo123"}' \
  http://localhost:8080/api/login
```

### Error Responses

The API returns standard HTTP status codes:
- **400 Bad Request**: Invalid input or validation errors
- **401 Unauthorized**: Invalid credentials
- **404 Not Found**: User not found
- **409 Conflict**: Username or email already exists
- **500 Internal Server Error**: Database or system errors

---

## Building and Testing (For Developers)

### Using the Setup Script (Recommended)

The easiest way to build and start the server:

```bash
./setup.sh
```

This script will:
- Check that you have all required tools (CMake, Make, G++, curl)
- Build the C++ backend automatically
- Start the HTTP server on port 8080
- Test all API endpoints
- Show troubleshooting information if needed

### Manual Build Process

If you prefer to build manually:

#### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler (GCC or Clang)
- SQLite3 development libraries
- OpenSSL development libraries
- Make (build-essential package)

#### Build Steps
```bash
mkdir build && cd build
cmake ..
make
cd ..
```

#### Run the Server
```bash
cd build
./sohbet
```

#### Test the API
```bash
curl http://localhost:8080/api/status
curl http://localhost:8080/api/users/demo
```

### Running Tests

To run the automated test suite:

```bash
cd build
ctest
# Or for verbose output:
ctest -V
```

### Full Stack Development

To run both backend and frontend together:

```bash
./start-fullstack.sh
```

This starts:
- C++ backend server on port 8080
- React frontend development server on port 3000
- Automatic proxy configuration for API calls

---

## Troubleshooting

Having issues? Here are common problems and solutions:

### Backend Server Issues

#### Problem: Frontend shows "ECONNREFUSED" error

**Solutions:**

1. **Verify the server is running:**
   ```bash
   curl http://localhost:8080/api/status
   # Should return: {"status":"ok","version":"0.3.0-academic",...}
   ```

2. **Check if port 8080 is in use:**
   ```bash
   lsof -i :8080
   # Should show sohbet process
   ```

3. **Restart the server:**
   ```bash
   pkill -f sohbet
   ./setup.sh
   ```

#### Problem: Server won't start

**Solutions:**

1. **Verify the executable exists:**
   ```bash
   ls -la build/sohbet
   # Should exist and be executable
   ```

2. **Check server logs:**
   ```bash
   cd build && ./sohbet
   # Watch for error messages
   ```

3. **Verify dependencies:**
   ```bash
   ldd build/sohbet
   ```

#### Problem: Permission denied

**Solution:**
```bash
chmod +x build/sohbet
chmod +x setup.sh
```

#### Problem: Port 8080 already in use

**Solutions:**
```bash
# Find what's using port 8080
lsof -i :8080

# Kill the process (replace PID with actual ID)
kill <PID>

# Or kill all sohbet processes
pkill -f sohbet
```

### Build Issues

#### Problem: CMake configuration fails

**Solutions:**

1. **Install missing dependencies:**
   
   On Ubuntu/Debian:
   ```bash
   sudo apt update
   sudo apt install cmake build-essential libsqlite3-dev libssl-dev
   ```
   
   On macOS (with Homebrew):
   ```bash
   brew install cmake sqlite openssl
   ```

2. **Check CMake version:**
   ```bash
   cmake --version  # Should be 3.16 or higher
   ```

3. **Clean and rebuild:**
   ```bash
   rm -rf build
   mkdir build && cd build
   cmake ..
   ```

#### Problem: Make build fails

**Solutions:**

1. **Check compiler version:**
   ```bash
   g++ --version  # Should support C++17
   ```

2. **Clean rebuild:**
   ```bash
   cd build
   make clean
   make
   ```

3. **Verbose build to see detailed errors:**
   ```bash
   make VERBOSE=1
   ```

### Frontend Issues

#### Problem: Frontend can't reach backend API

**Verification Steps:**

1. Backend server is running: `curl http://localhost:8080/api/status`
2. Frontend proxy is configured: Check `frontend/package.json` for `"proxy": "http://localhost:8080"`
3. CORS headers are present: Server should return `Access-Control-Allow-Origin: *`

**Solution:**

Use the full stack startup script which handles all configuration:
```bash
./start-fullstack.sh
```

### Testing the Complete Setup

Run these commands to verify everything is working:

```bash
# Test all endpoints
curl -v http://localhost:8080/api/status
curl -v http://localhost:8080/api/users/demo

# Test user registration
curl -v -X POST -H "Content-Type: application/json" \
  -d '{"username":"test","email":"test@example.edu","password":"TestPass123"}' \
  http://localhost:8080/api/users

# Check server health (with formatted output)
curl -s http://localhost:8080/api/status | python3 -m json.tool
```

---

## Technical Architecture (For Developers)

### System Components

Sohbet is built with a clean separation of concerns:

- **Models**: User data structures with academic profile fields and JSON serialization
- **Database**: SQLite with RAII wrapper, migrations, and safe query operations
- **Repositories**: Data access layer providing CRUD operations
- **Security**: bcrypt password hashing and JWT token management
- **Server**: HTTP request handlers and API endpoint routing
- **Utils**: Hashing utilities and input validation helpers
- **Password Hashing**: Production-ready bcrypt with 12 rounds
- **Authentication**: JWT tokens with HMAC-SHA256 signatures
- **Data Protection**: No sensitive data in API responses
- **Input Validation**: Comprehensive validation and sanitization

**Security Implementation**: The backend uses industry-standard bcrypt (via libbcrypt) for secure password hashing and JWT (HS256) for stateless authentication. All security tests pass and verify no password leakage in API responses.

## Test Coverage ✅

Comprehensive unit and integration tests ensure authentication security and correctness.

### Authentication Tests (`tests/test_authentication.cpp`)

**User Registration**
- ✅ Successful registration returns user data with ID
- ✅ Password hashes are never exposed in API responses
- ✅ User data properly serialized without sensitive fields

**User Login**
- ✅ Successful login with valid credentials returns JWT token and user data
- ✅ Failed login with wrong password returns 401 error
- ✅ Failed login with non-existent user returns 401 error
- ✅ Error responses never contain tokens

**JWT Token Management**
- ✅ Token generation creates valid JWT with correct structure
- ✅ Token verification decodes and validates payload (username, user_id)
- ✅ Invalid tokens are properly rejected
- ✅ Token expiration is set and enforced

**Demo User**
- ✅ Demo user auto-created on server initialization
- ✅ Login with demo credentials (demo_student / demo123) succeeds
- ✅ Login with wrong demo password fails with 401

### Other Test Suites
- **BcryptTest**: Password hashing, verification, and legacy compatibility
- **UserTest**: User model validation and JSON serialization
- **UserRepositoryTest**: Database CRUD operations and uniqueness constraints
- **VoiceServiceTest**: Voice channel configuration and token generation

### Running Tests
```bash
cd build
ctest --output-on-failure
```

All tests are registered in CMakeLists.txt and run via CTest. Tests use in-memory databases to ensure isolation and repeatability.

## Architecture

### Project Structure

```
Sohbet/
├── include/              # Header files
│   ├── models/
│   │   └── user.h
│   ├── db/
│   │   └── database.h
│   ├── repositories/
│   │   └── user_repository.h
│   ├── security/
│   │   ├── bcrypt_wrapper.h
│   │   └── jwt.h
│   ├── server/
│   │   └── server.h
│   └── utils/
│       └── hash.h
├── src/                  # Source files
│   ├── models/
│   │   └── user.cpp
│   ├── db/
│   │   └── database.cpp
│   ├── repositories/
│   │   └── user_repository.cpp
│   ├── security/
│   │   ├── bcrypt_wrapper.cpp
│   │   └── jwt.cpp
│   ├── server/
│   │   └── server.cpp
│   ├── utils/
│   │   └── hash.cpp
│   └── main.cpp
├── tests/               # Test files
│   ├── test_user.cpp
│   ├── test_user_repository.cpp
│   ├── test_bcrypt.cpp
│   └── test_authentication.cpp
├── frontend/            # React frontend
│   └── src/
└── docs/                # Additional documentation

```

### Dependencies

The project uses these external libraries:

- **SQLite3**: Lightweight database for data storage
- **bcrypt**: Secure password hashing (fetched via CMake)
- **OpenSSL**: HMAC-SHA256 signatures for JWT tokens

### Migration History

**Backend Evolution**: The backend has been fully migrated from Node.js to pure C++17:

- ✅ Removed all Node.js dependencies (Express.js, package.json, etc.)
- ✅ Implemented custom HTTP server with socket programming
- ✅ Created SQLite integration with RAII patterns
- ✅ Added bcrypt password hashing and JWT authentication
- ✅ Built comprehensive REST API
- ✅ Set up CMake build system
- ✅ Created full test suite

The frontend remains React+TypeScript and communicates with the C++ backend via REST API.

---

## Roadmap - Implementation Status

### Phase 1: Foundation & Authentication ✅ COMPLETE (100%)

- ✅ User registration with validation and uniqueness checks
- ✅ Secure password hashing (bcrypt, 12 rounds)
- ✅ User authentication with JWT token generation
- ✅ SQLite database integration with RAII wrapper
- ✅ Academic profile fields (username, email, university, department, year, language)
- ✅ Profile photos and media upload
- ✅ Role-based access control (Students, Professors, Admins)

### Phase 2: Academic Networking ✅ COMPLETE (100%)

- ✅ Friend request system (send, accept, reject, unfriend)
- ✅ Friends list and management
- ✅ Post creation with visibility controls
- ✅ Post feed (personalized and global)
- ✅ Reaction system (like, love, insightful)
- ✅ Comments and threaded replies
- ✅ Student-to-student connections

### Phase 3: Groups & Organizations ✅ COMPLETE (100%)

- ✅ Study group creation (professor-only feature)
- ✅ Group membership management
- ✅ Group browsing and discovery
- ✅ Organization/club creation
- ✅ Organization directory and profiles
- ✅ Organization account management

### Phase 4: Real-Time Communication ✅ COMPLETE (95%)

#### Phase 4A: Real-Time Chat ✅ COMPLETE (100%)
- ✅ WebSocket infrastructure for real-time communication
- ✅ Conversation creation and management
- ✅ Real-time message delivery
- ✅ Typing indicators
- ✅ Online/offline presence tracking
- ✅ Message history with pagination
- ✅ Automatic reconnection handling

#### Phase 4C: Voice Channels ✅ API COMPLETE (90%)
- ✅ Voice channel database models
- ✅ Voice channel REST API endpoints
- ✅ Channel creation and management
- ✅ Session tracking (join/leave)
- ✅ Active user counting
- ⚠️ Murmur server integration (pending)
- ⚠️ WebRTC voice streaming (pending)
- ⚠️ Full Khave UI implementation (in progress)

**See [WEBSOCKET_INFRASTRUCTURE.md](WEBSOCKET_INFRASTRUCTURE.md) for detailed WebSocket documentation.**
**See [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md) for complete implementation details.**

### Future Enhancements (Planned)

- 📝 Email verification for accounts
- 📝 Rate limiting and enhanced security
- 📝 File sharing in messages and posts
- 📝 Advanced search and filtering
- 📝 Analytics and insights dashboard
- 📝 Course enrollment management
- 📝 Q&A system for academic help
- 📝 Screen sharing in voice channels
- 📝 Video calling support

---

## Contributing

Sohbet is an **academic project** designed for learning C++ backend development and security practices. 

**We welcome contributions**, but please keep in mind:
- This project is focused on educational purposes
- It's not intended for production deployment without additional security hardening
- Contributions should maintain the educational value of the codebase

**How to Contribute:**
1. Fork the repository
2. Create a feature branch
3. Make your changes with clear commit messages
4. Test your changes thoroughly
5. Submit a pull request with a description of your changes

---

## License and Usage

**Educational Use Only**

This project is intended for educational and learning purposes. It demonstrates modern software development practices but is **not production-ready** without additional security auditing and hardening.

**Key Points:**
- ✅ Free to use for learning and educational projects
- ✅ Feel free to study and modify the code
- ⚠️ Not recommended for production deployment without security review
- ⚠️ Use at your own risk

---

## Additional Resources

### Documentation Files

- **[IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md)** - Complete implementation roadmap with current status
- **[ROADMAP_STATUS_CHECK.md](ROADMAP_STATUS_CHECK.md)** - Detailed status analysis of implementation progress
- **[WEBSOCKET_INFRASTRUCTURE.md](WEBSOCKET_INFRASTRUCTURE.md)** - WebSocket real-time communication documentation
- **[PHASE2_SUMMARY.md](PHASE2_SUMMARY.md)** - Phase 2 (Social Features) completion report
- **[PHASE3_SUMMARY.md](PHASE3_SUMMARY.md)** - Phase 3 (Groups/Organizations) completion report
- **[PHASE4_SUMMARY.md](PHASE4_SUMMARY.md)** - Phase 4 initial completion report
- **[PHASE4A_COMPLETION_REPORT.md](PHASE4A_COMPLETION_REPORT.md)** - Phase 4A (Real-Time Chat) completion report
- **[PHASE4C_COMPLETION_REPORT.md](PHASE4C_COMPLETION_REPORT.md)** - Phase 4C (Voice Channels API) completion report
- **[3rd-Party Service Integration.md](3rd-Party Service Integration.md)** - Information about voice/video integration with Murmur
- **[INTEGRATION_SUMMARY.md](INTEGRATION_SUMMARY.md)** - Summary of the Murmur integration implementation
- **[docs/ACADEMIC_FEATURES.md](docs/ACADEMIC_FEATURES.md)** - Details about academic features and future enhancements
- **[docs/VOICE_INTEGRATION.md](docs/VOICE_INTEGRATION.md)** - Developer guide for voice integration
- **[frontend/README.md](frontend/README.md)** - Frontend-specific documentation

### Getting Help

If you run into issues or have questions:

1. **Check the Troubleshooting section** above
2. **Review the documentation files** for detailed information
3. **Check existing GitHub issues** for similar problems
4. **Create a new issue** with:
   - Clear description of the problem
   - Steps to reproduce
   - Your system information (OS, compiler version, etc.)
   - Error messages or logs

### Contact

This is an open-source educational project. For questions or collaboration:
- Open an issue on GitHub
- Check the documentation
- Review the code and tests for examples

---

**Thank you for your interest in Sohbet!** 🎓
