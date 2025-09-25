# Sohbet Frontend

React frontend for the Sohbet Academic Social Platform.

## Technologies Used

- **React 18** with TypeScript
- **Material-UI (MUI)** for component library
- **Axios** for API communication
- **React Router** for navigation

## Features

- **User Authentication**: Login and registration forms
- **User Dashboard**: Profile display with academic information
- **Backend Integration**: Real-time communication with C++ backend
- **Status Display**: Live server status and demo user data
- **Responsive Design**: Mobile-friendly interface

## Development Setup

### Prerequisites

- Node.js 16+ and npm
- C++ backend server running on port 8080

### Installation

1. Install dependencies:
```bash
npm install
```

2. Start development server:
```bash
npm start
```

The frontend will be available at `http://localhost:3000` and will proxy API requests to the backend on `http://localhost:8080`.

### Available Scripts

- `npm start` - Start development server
- `npm run build` - Build for production
- `npm test` - Run tests
- `npm run eject` - Eject from Create React App

## Backend Integration

The frontend communicates with the C++ backend through REST API endpoints:

- `GET /api/status` - Server status and version
- `GET /api/users/demo` - Demo user data
- `POST /api/users` - User registration
- `POST /api/login` - User authentication

## Components

### LoginForm
Handles user authentication with username/password.

### RegisterForm
Comprehensive registration form with academic profile fields:
- Username, email, password
- University, department, enrollment year
- Primary and additional languages

### Dashboard
User profile display showing:
- User information and avatar
- Academic details
- Platform features and roadmap

### StatusDisplay
Real-time status of:
- Backend server health
- Available features
- Demo user data
- Connection status

## Project Structure

```
frontend/
├── public/
│   └── index.html
├── src/
│   ├── components/
│   │   ├── LoginForm.tsx
│   │   ├── RegisterForm.tsx
│   │   ├── Dashboard.tsx
│   │   └── StatusDisplay.tsx
│   ├── services/
│   │   └── api.ts
│   ├── App.tsx
│   ├── App.css
│   └── index.tsx
├── package.json
└── tsconfig.json
```

## Development Notes

- The app uses localStorage to persist authentication tokens
- Error handling is implemented for all API calls
- Loading states are shown during async operations
- The interface supports both Turkish and English academic environments