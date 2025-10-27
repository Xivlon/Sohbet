# Sohbet - Academic Social Platform

## Overview
Sohbet is a Next.js-based social media platform designed for academic discussions and collaboration. It provides a modern, responsive interface for academic users to connect, share posts, join groups, and communicate. The platform aims to foster a collaborative environment for researchers, professors, engineers, and students.

## User Preferences
I prefer simple language and direct instructions. I want iterative development, where features are built step-by-step. Ask before making major changes to the project structure or core functionalities. I appreciate detailed explanations for complex implementations.

## System Architecture

### Frontend
- **Framework**: Next.js 16.0.0 with React 19.2.0
- **Styling**: Tailwind CSS 4 with Radix UI components
- **UI/UX**:
    - **Color Schemes**: Red and white (light mode) for academic aesthetic; Black and gray (dark mode) for night reading.
    - **Responsive Design**: Desktop (left sidebar, detailed navigation), Mobile (bottom navigation, compact icons).
    - **Feed Layout**: Adapts to screen size with a `max-width: 768px` breakpoint.
- **Key Features**: Authentication-gated content, login/registration UI with Turkish localization, profile customization (view/edit modes), optimistic UI updates for reactions.
- **Build Tool**: Turbopack
- **Package Manager**: npm

### Backend
- **Language**: C++17 with Clang 14
- **Build System**: CMake 3.16+
- **Database**: SQLite3 with custom ORM
- **Authentication**: JWT token-based, bcrypt password hashing.
- **HTTP Server**: Custom HTTP/1.1 implementation on port 8080.
- **Architecture**: Dual-workflow (Frontend on port 5000, Backend API on port 8080).
- **Security**: JWT secret managed via environment variables (`SOHBET_JWT_SECRET`), CORS enabled, RBAC for permissions (Student, Professor, Admin roles).
- **Data Model**: Extended User model with academic fields (name, position, phone_number, university, department, enrollment_year, primary_language).
- **Database Schema**: 18 tables including users, roles, friendships, posts, comments, groups, organizations, conversations, messages, and voice channels.

### Full-Stack Integration
- **Communication**: Frontend uses an API client utility (`frontend/app/lib/api-client.ts`) to interact with the C++ backend.
- **Authentication Flow**: Login/registration via `AuthModal`, JWT token storage in localStorage, token included in API requests for session persistence.
- **Environment**: Configured for Replit with Next.js binding to `0.0.0.0:5000` and backend to `0.0.0.0:8080`.

## External Dependencies

### Frontend
- **UI Components**: Radix UI (dialog, label, skeleton, dropdown-menu, scroll-area, select, slider, tabs)
- **Icons**: Lucide React
- **Theming**: `next-themes`
- **Styling Utilities**: `class-variance-authority`, `tailwind-merge`

### Backend
- **Database**: SQLite3
- **Cryptography**: OpenSSL (for JWT), `libbcrypt` (for bcrypt)
- **Build**: `pkg-config`

### Development/Tools
- **Python**: Python 3.11 for data import scripts.