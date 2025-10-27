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

### Backend Components (C++)
The project also includes a C++ backend with:
- Voice integration using Mumble (murmur)
- Database layer with SQLite
- User authentication with bcrypt and JWT
- RESTful server implementation

## Development

### Running the Application
The Next.js development server is configured to run automatically:
- **Port**: 5000
- **Host**: 0.0.0.0 (required for Replit compatibility)
- **Command**: `cd frontend && npm run dev`

### Scripts Available
- `npm run dev`: Start development server on port 5000
- `npm run build`: Build production bundle
- `npm run start`: Start production server on port 5000
- `npm run lint`: Run ESLint

### Environment
- **Node.js**: Version 20
- **Dependencies**: Automatically installed via npm

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
