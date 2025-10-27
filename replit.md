# Sohbet - Academic Social Platform

## Overview
Sohbet is a Next.js-based social media platform designed for academic discussions and collaboration. The frontend provides a modern, responsive interface built with React 19, Next.js 16, and Tailwind CSS.

**Current State**: Successfully migrated from Vercel to Replit and running in development mode.

## Recent Changes
- **October 27, 2025**: Migrated from Vercel to Replit
  - Configured Next.js to bind to 0.0.0.0:5000 for Replit networking compatibility
  - Installed all npm dependencies (414 packages)
  - Set up development workflow with auto-restart
  - Configured deployment settings for Autoscale deployment target
  - Updated .gitignore with Next.js-specific patterns
  - Updated color scheme:
    - Light mode: Red and white color palette for a clean, academic aesthetic
    - Dark mode: Black and gray color palette for comfortable night reading
  - Implemented device detection for optimized layouts:
    - Desktop: Left sidebar navigation with full labels and descriptions, enhanced header
    - Mobile: Bottom navigation bar with compact icons, streamlined header
    - Responsive feed layout that adapts to screen size (max-width 768px breakpoint)
  - **Backend Integration Completed**:
    - Compiled C++ backend server with CMake
    - Set up dual-workflow architecture: Frontend (port 5000) + Backend API (port 8080)
    - Implemented full authentication system with JWT tokens
    - Created API client utilities for frontend-backend communication
    - Added login/registration UI components with Turkish localization
  - **User Data Import System**:
    - Extended User model with professional academic fields: name, position, phone_number, created_at, warnings
    - Updated database schema to support complete academic profiles
    - Created Python import script for bulk user data loading
    - Imported 25 Turkish academic users (researchers, professors, engineers, students)
    - Implemented paginated user listing API with query parameter support
  - **Authentication-Gated Content Access**:
    - Implemented mandatory login before content access
    - Login page appears first for all unauthenticated users
    - AuthModal cannot be closed until user successfully authenticates
    - Full app content (feed, sidebar, navigation) only visible after login
    - Loading state shown during authentication verification

## Project Architecture

### Frontend Structure
- **Location**: `frontend/` directory
- **Framework**: Next.js 16.0.0 with React 19.2.0
- **Styling**: Tailwind CSS 4 with Radix UI components
- **Build Tool**: Turbopack (Next.js built-in)
- **Package Manager**: npm

### Key Technologies
- **UI Components**: Radix UI (scroll-area, select, slider, tabs)
- **Icons**: Lucide React
- **Theming**: next-themes for dark/light mode
- **Styling Utilities**: class-variance-authority, tailwind-merge

### Backend Structure
- **Location**: `src/`, `include/`, `build/` directories
- **Language**: C++17 with Clang 14
- **Build System**: CMake 3.16+
- **Database**: SQLite3 with custom ORM
- **Authentication**: bcrypt password hashing + JWT token-based auth
- **HTTP Server**: Custom HTTP/1.1 server implementation on port 8080

### Backend API Endpoints
- `GET /api/status` - Server health check
- `GET /api/users?limit=50&offset=0` - List all users with pagination (default limit: 50, max: 100)
- `GET /api/users/demo` - Demo user data
- `POST /api/users` - User registration
- `POST /api/login` - User authentication (returns JWT token)

### User Data Model
Each user profile includes:
- **Core Fields**: id, username, email, password_hash
- **Personal Info**: name (full name)
- **Professional Info**: position (Researcher, Professor, Engineer, Technician, Student), phone_number
- **Academic Info**: university, department, enrollment_year
- **Metadata**: created_at, warnings, primary_language, additional_languages

### Sample User Data
The database contains 27 users including:
- 2 demo/test users (demo_student, ali_student)
- 25 imported Turkish academics from major universities (Istanbul University, Ankara University, Ege University, Bilkent University, Marmara University)
- Diverse positions: Professors, Researchers, Engineers, Technicians, Students
- Specializations: Mathematics, Computer Science, Biology, Physics, Chemistry, Mechanical Engineering

### Full-Stack Integration
- **Frontend-Backend Communication**: 
  - API client utility (`frontend/app/lib/api-client.ts`) handles all HTTP requests
  - Authentication context (`frontend/app/contexts/auth-context.tsx`) manages user session
  - Environment variable `NEXT_PUBLIC_API_URL` configures backend URL (default: http://localhost:8080)
- **Authentication Flow**:
  1. User registers/logs in via AuthModal component
  2. Backend validates credentials and returns JWT token
  3. Frontend stores token in localStorage and includes it in API requests
  4. User session persists across page reloads
- **Security**: CORS enabled on backend, bcrypt for password hashing, JWT for stateless auth

### C++ Dependencies
- **SQLite3**: Database operations
- **OpenSSL**: Cryptographic functions for JWT
- **bcrypt**: Password hashing (via libbcrypt)
- **pkg-config**: Build dependency resolution

## Development

### Running the Application
Two workflows run concurrently:

**1. Frontend (Next.js)**
- **Port**: 5000
- **Host**: 0.0.0.0 (required for Replit compatibility)
- **Command**: `cd frontend && npm run dev`
- **Environment**: Node.js 20, automatically loads `.env.local`

**2. Backend API (C++)**
- **Port**: 8080
- **Host**: 0.0.0.0
- **Command**: `cd build && ./sohbet`
- **Database**: SQLite file at `build/academic.db`

### Frontend Scripts
- `npm run dev`: Start development server on port 5000
- `npm run build`: Build production bundle
- `npm run start`: Start production server on port 5000
- `npm run lint`: Run ESLint

### Backend Build
```bash
mkdir -p build && cd build
cmake ..
make sohbet -j$(nproc)
./sohbet  # Starts server on port 8080
```

### Data Import
To import user data from SQL files:
```bash
python3 scripts/import_users.py
```
Default password for all imported users: `[username]123`  
Example: User `ahmet_yilmaz` has password `ahmet_yilmaz123`

### System Dependencies
- **C++ Toolchain**: cpp-clang14
- **Build Tools**: cmake, pkg-config
- **Libraries**: sqlite, openssl
- **Python**: Python 3.11 with bcrypt for data import scripts

## Deployment Configuration
- **Target**: Autoscale (stateless deployment)
- **Build**: `cd frontend && npm run build`
- **Start**: `cd frontend && npm run start`

## Notes
- The cross-origin warnings in development logs are informational only and don't affect functionality
- The application is configured for Replit's networking environment
- All dependencies installed successfully with no vulnerabilities

## Future Enhancements
- **Cross-Origin Configuration**: Next.js 16 displays informational warnings about `allowedDevOrigins`. In a future major version of Next.js, you may need to explicitly configure this setting in `next.config.ts` to match Replit preview hosts. This is not urgent as it currently works without issues, but should be addressed before upgrading to Next.js 17+.
