# Sohbet Frontend

The React-based user interface for the Sohbet Academic Social Platform.

---

## What's Inside

This is the frontend (user interface) of Sohbet, built with modern web technologies to provide a smooth, responsive experience.

### Technologies Used

- **⚛️ React 18** - Modern UI library with TypeScript for type safety
- **🎨 Material-UI (MUI)** - Beautiful, professional UI components
- **🔌 Axios** - Easy API communication with the backend
- **🗺️ React Router** - Smooth navigation between pages

---

## What You Can Do

### Current Features

- ✅ **User Authentication** - Login and registration with validation
- ✅ **User Dashboard** - View and manage your profile
- ✅ **Backend Integration** - Real-time communication with the C++ server
- ✅ **Status Display** - See server health and demo user information
- ✅ **Responsive Design** - Works on desktop, tablet, and mobile

### Coming Soon

- 📝 Study groups and course enrollment
- 📝 Academic discussions and Q&A
- 📝 Real-time messaging
- 📝 Voice and video calls

---

## Getting Started

### What You Need

Before you begin, make sure you have:
- **Node.js 16 or higher** with npm
- **The C++ backend server** running on port 8080

### Installation Steps

1. **Install dependencies**:
   ```bash
   npm install
   ```

2. **Start the development server**:
   ```bash
   npm start
   ```

3. **Open in your browser**:
   - The app will automatically open at `http://localhost:3000`
   - API requests are automatically forwarded to the backend at `http://localhost:8080`

### Available Commands

| Command | What It Does |
|---------|--------------|
| `npm start` | Starts the development server with hot reload |
| `npm run build` | Creates an optimized production build |
| `npm test` | Runs the test suite |
| `npm run eject` | Ejects from Create React App (⚠️ irreversible) |

---

## How It Works

### Backend Communication

The frontend talks to the C++ backend using REST API endpoints:

| Endpoint | Purpose |
|----------|---------|
| `GET /api/status` | Check server health and version |
| `GET /api/users/demo` | Get demo user information |
| `POST /api/users` | Register a new user |
| `POST /api/login` | Authenticate and get a token |
| `GET /api/users/:username` | Get user profile |

All API calls are automatically authenticated using JWT tokens stored in your browser.

---

## Main Components

### 🔐 LoginForm
Handles user authentication with username and password.

**Features**:
- Input validation
- Error handling
- Token storage in localStorage
- Automatic redirect after login

### 📝 RegisterForm
Comprehensive registration form for new users.

**Academic Fields**:
- Username, email, password
- University and department
- Enrollment year
- Primary and additional languages

**Features**:
- Real-time validation
- Password strength checking
- Duplicate username/email detection

### 📊 Dashboard
Your personal profile page.

**Shows**:
- User information and avatar
- Academic details (university, department, year)
- Platform features and roadmap
- Navigation to other sections

### 📡 StatusDisplay
Real-time system information.

**Displays**:
- Backend server health
- Available features
- Demo user credentials
- Connection status

---

## Project Structure

Here's how the frontend code is organized:

```
frontend/
├── public/              # Static files
│   └── index.html       # Main HTML template
│
├── src/                 # Source code
│   ├── components/      # React components
│   │   ├── LoginForm.tsx
│   │   ├── RegisterForm.tsx
│   │   ├── Dashboard.tsx
│   │   └── StatusDisplay.tsx
│   │
│   ├── services/        # API and business logic
│   │   ├── api.ts       # Backend API client
│   │   └── voiceService.ts  # Voice integration
│   │
│   ├── hooks/           # Custom React hooks
│   │   └── useVoice.ts  # Voice channel hooks
│   │
│   ├── App.tsx          # Main app component
│   ├── App.css          # Global styles
│   └── index.tsx        # Entry point
│
├── package.json         # Dependencies and scripts
└── tsconfig.json        # TypeScript configuration
```

---

## Development Tips

### Authentication

The app uses **localStorage** to persist authentication tokens:

```typescript
// After successful login
localStorage.setItem('token', jwtToken);

// Access the token for API calls
const token = localStorage.getItem('token');
```

### Error Handling

All API calls include comprehensive error handling:

```typescript
try {
  const response = await api.post('/api/login', credentials);
  // Handle success
} catch (error) {
  // Handle error with user-friendly message
  console.error('Login failed:', error);
}
```

### Loading States

Components show loading indicators during async operations:

```typescript
const [loading, setLoading] = useState(false);

const handleSubmit = async () => {
  setLoading(true);
  try {
    // API call
  } finally {
    setLoading(false);
  }
};
```

### Language Support

The interface is designed for both Turkish and English academic environments, with easy-to-read labels and internationalization-ready structure.

---

## Troubleshooting

### Frontend Can't Connect to Backend

**Problem**: API calls fail with connection errors

**Solution**:
1. Make sure the backend is running: `curl http://localhost:8080/api/status`
2. Check that `package.json` has: `"proxy": "http://localhost:8080"`
3. Restart the frontend: `npm start`

### Build Errors

**Problem**: `npm install` or `npm start` fails

**Solution**:
1. Delete `node_modules` and `package-lock.json`
2. Run `npm install` again
3. Check Node.js version: `node --version` (should be 16+)

### Browser Console Errors

**Problem**: JavaScript errors in the browser

**Solution**:
1. Open browser DevTools (F12)
2. Check the Console tab for error details
3. Clear browser cache and reload
4. Check that the backend is running properly

---

## Contributing

When adding new components or features:

1. ✅ Follow the existing TypeScript patterns
2. ✅ Use Material-UI components for consistency
3. ✅ Add proper error handling
4. ✅ Include loading states
5. ✅ Test with both demo and real user data
6. ✅ Ensure responsive design on mobile

---

## Learn More

- [React Documentation](https://react.dev)
- [Material-UI Documentation](https://mui.com)
- [TypeScript Documentation](https://www.typescriptlang.org/docs)
- [Main Sohbet README](../README.md) - For overall project information

---

**Happy coding!** 🚀