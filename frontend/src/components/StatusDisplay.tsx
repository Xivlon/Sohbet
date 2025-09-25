import React, { useState, useEffect } from 'react';
import {
  Box,
  Paper,
  Typography,
  Chip,
  Alert,
  Grid,
  Card,
  CardContent,
  CircularProgress,
  Avatar,
} from '@mui/material';
import {
  CheckCircle,
  Info,
  Person,
  School,
} from '@mui/icons-material';
import ApiService from '../services/api';

interface ServerStatus {
  status: string;
  version: string;
  features: string[];
}

interface DemoUser {
  id?: number;
  username: string;
  email: string;
  university?: string;
  department?: string;
  enrollment_year?: number;
  primary_language?: string;
}

const StatusDisplay: React.FC = () => {
  const [status, setStatus] = useState<ServerStatus | null>(null);
  const [demoUser, setDemoUser] = useState<DemoUser | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [statusData, demoUserData] = await Promise.all([
          ApiService.getStatus(),
          ApiService.getDemoUser(),
        ]);
        setStatus(statusData);
        setDemoUser(demoUserData);
      } catch (err) {
        console.error('Failed to fetch status data:', err);
        setError('Failed to connect to backend server');
      } finally {
        setLoading(false);
      }
    };

    fetchData();
  }, []);

  if (loading) {
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', p: 3 }}>
        <CircularProgress />
      </Box>
    );
  }

  if (error) {
    return (
      <Alert severity="error" sx={{ mt: 2 }}>
        {error}
      </Alert>
    );
  }

  return (
    <Box sx={{ mt: 4 }}>
      <Typography variant="h5" component="h2" gutterBottom>
        System Status
      </Typography>

      <Grid container spacing={3}>
        {/* Server Status */}
        {status && (
          <Grid item xs={12} md={6}>
            <Card elevation={2}>
              <CardContent>
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  <CheckCircle color="success" sx={{ mr: 1 }} />
                  <Typography variant="h6">
                    Backend Server
                  </Typography>
                </Box>

                <Typography variant="body2" color="text.secondary" gutterBottom>
                  Status: <strong>{status.status}</strong>
                </Typography>
                
                <Typography variant="body2" color="text.secondary" gutterBottom>
                  Version: <strong>{status.version}</strong>
                </Typography>

                <Typography variant="body2" color="text.secondary" sx={{ mb: 2 }}>
                  Available Features:
                </Typography>

                <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1 }}>
                  {status.features.map((feature, index) => (
                    <Chip
                      key={index}
                      label={feature.replace(/_/g, ' ')}
                      size="small"
                      color="primary"
                      variant="outlined"
                    />
                  ))}
                </Box>
              </CardContent>
            </Card>
          </Grid>
        )}

        {/* Demo User */}
        {demoUser && (
          <Grid item xs={12} md={6}>
            <Card elevation={2}>
              <CardContent>
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  <Info color="info" sx={{ mr: 1 }} />
                  <Typography variant="h6">
                    Demo User Data
                  </Typography>
                </Box>

                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  <Avatar sx={{ width: 40, height: 40, mr: 2, bgcolor: 'secondary.main' }}>
                    {demoUser.username.charAt(0).toUpperCase()}
                  </Avatar>
                  <Box>
                    <Typography variant="body1" fontWeight="bold">
                      {demoUser.username}
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      ID: {demoUser.id}
                    </Typography>
                  </Box>
                </Box>

                <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                  <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                    <Person sx={{ fontSize: 16 }} color="action" />
                    <Typography variant="body2">
                      {demoUser.email}
                    </Typography>
                  </Box>

                  {demoUser.university && (
                    <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                      <School sx={{ fontSize: 16 }} color="action" />
                      <Typography variant="body2">
                        {demoUser.university}
                      </Typography>
                    </Box>
                  )}

                  {demoUser.department && (
                    <Typography variant="body2" color="text.secondary">
                      Department: {demoUser.department}
                    </Typography>
                  )}

                  {demoUser.enrollment_year && (
                    <Typography variant="body2" color="text.secondary">
                      Enrolled: {demoUser.enrollment_year}
                    </Typography>
                  )}

                  {demoUser.primary_language && (
                    <Typography variant="body2" color="text.secondary">
                      Language: {demoUser.primary_language}
                    </Typography>
                  )}
                </Box>
              </CardContent>
            </Card>
          </Grid>
        )}
      </Grid>

      <Alert severity="info" sx={{ mt: 3 }}>
        <Typography variant="body2">
          <strong>Backend Integration Active:</strong> This React frontend is successfully communicating with the C++ backend server running on port 8080.
          The backend provides REST API endpoints for user management, authentication, and academic social features.
        </Typography>
      </Alert>
    </Box>
  );
};

export default StatusDisplay;