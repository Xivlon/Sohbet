import React, { useState, useEffect } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import { CssBaseline, Container, AppBar, Toolbar, Typography, Button, Box } from '@mui/material';
import LoginForm from './components/LoginForm';
import RegisterForm from './components/RegisterForm';
import Dashboard from './components/Dashboard';
import StatusDisplay from './components/StatusDisplay';
import './App.css';

// Create a theme for the Turkish academic social platform
const theme = createTheme({
  palette: {
    primary: {
      main: '#1976d2', // Blue
    },
    secondary: {
      main: '#dc004e', // Red (Turkish flag colors)
    },
    background: {
      default: '#f5f5f5',
    },
  },
  typography: {
    h1: {
      fontSize: '2.5rem',
      fontWeight: 600,
    },
    h4: {
      fontSize: '1.5rem',
      fontWeight: 500,
    },
  },
});

interface User {
  id: number;
  username: string;
  email: string;
  university?: string;
  department?: string;
  enrollment_year?: number;
  primary_language?: string;
}

function App() {
  const [user, setUser] = useState<User | null>(null);
  const [token, setToken] = useState<string | null>(null);
  const [currentView, setCurrentView] = useState<'login' | 'register' | 'dashboard'>('login');

  useEffect(() => {
    // Check for existing token in localStorage
    const savedToken = localStorage.getItem('sohbet_token');
    const savedUser = localStorage.getItem('sohbet_user');
    
    if (savedToken && savedUser) {
      setToken(savedToken);
      setUser(JSON.parse(savedUser));
      setCurrentView('dashboard');
    }
  }, []);

  const handleLogin = (userData: User, authToken: string) => {
    setUser(userData);
    setToken(authToken);
    setCurrentView('dashboard');
    
    // Save to localStorage
    localStorage.setItem('sohbet_token', authToken);
    localStorage.setItem('sohbet_user', JSON.stringify(userData));
  };

  const handleLogout = () => {
    setUser(null);
    setToken(null);
    setCurrentView('login');
    
    // Clear localStorage
    localStorage.removeItem('sohbet_token');
    localStorage.removeItem('sohbet_user');
  };

  const handleRegisterSuccess = (userData: User) => {
    // After successful registration, show login form
    setCurrentView('login');
  };

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <div className="App">
        <AppBar position="static">
          <Toolbar>
            <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
              Sohbet - Academic Social Platform
            </Typography>
            {user && (
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
                <Typography variant="body2">
                  Welcome, {user.username}
                </Typography>
                <Button color="inherit" onClick={handleLogout}>
                  Logout
                </Button>
              </Box>
            )}
          </Toolbar>
        </AppBar>

        <Container maxWidth="lg" sx={{ mt: 4, mb: 4 }}>
          {currentView === 'login' && !user && (
            <Box>
              <LoginForm onLogin={handleLogin} />
              <Box sx={{ mt: 2, textAlign: 'center' }}>
                <Button 
                  onClick={() => setCurrentView('register')}
                  variant="text"
                >
                  Don't have an account? Register here
                </Button>
              </Box>
            </Box>
          )}

          {currentView === 'register' && !user && (
            <Box>
              <RegisterForm onRegisterSuccess={handleRegisterSuccess} />
              <Box sx={{ mt: 2, textAlign: 'center' }}>
                <Button 
                  onClick={() => setCurrentView('login')}
                  variant="text"
                >
                  Already have an account? Login here
                </Button>
              </Box>
            </Box>
          )}

          {currentView === 'dashboard' && user && (
            <Dashboard user={user} token={token} />
          )}

          {/* Always show status display at the bottom */}
          <Box sx={{ mt: 4 }}>
            <StatusDisplay />
          </Box>
        </Container>
      </div>
    </ThemeProvider>
  );
}

export default App;