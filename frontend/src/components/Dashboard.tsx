import React from 'react';
import {
  Box,
  Paper,
  Typography,
  Grid,
  Chip,
  Avatar,
  Card,
  CardContent,
} from '@mui/material';
import {
  School,
  Person,
  Email,
  Language,
  CalendarToday,
} from '@mui/icons-material';

interface User {
  id?: number;
  username: string;
  email: string;
  university?: string;
  department?: string;
  enrollment_year?: number;
  primary_language?: string;
  additional_languages?: string[];
}

interface DashboardProps {
  user: User;
  token: string | null;
}

const Dashboard: React.FC<DashboardProps> = ({ user }) => {
  return (
    <Box>
      <Typography variant="h4" component="h1" gutterBottom>
        Welcome to Sohbet!
      </Typography>
      
      <Grid container spacing={3}>
        {/* User Profile Card */}
        <Grid item xs={12} md={6}>
          <Card elevation={3}>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 3 }}>
                <Avatar sx={{ width: 64, height: 64, mr: 2, bgcolor: 'primary.main' }}>
                  {user.username.charAt(0).toUpperCase()}
                </Avatar>
                <Box>
                  <Typography variant="h5" component="h2">
                    {user.username}
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    ID: {user.id}
                  </Typography>
                </Box>
              </Box>

              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Email color="action" />
                  <Typography variant="body1">
                    {user.email}
                  </Typography>
                </Box>

                {user.university && (
                  <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                    <School color="action" />
                    <Typography variant="body1">
                      {user.university}
                    </Typography>
                  </Box>
                )}

                {user.department && (
                  <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                    <Person color="action" />
                    <Typography variant="body1">
                      {user.department}
                    </Typography>
                  </Box>
                )}

                {user.enrollment_year && (
                  <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                    <CalendarToday color="action" />
                    <Typography variant="body1">
                      Enrolled: {user.enrollment_year}
                    </Typography>
                  </Box>
                )}

                {user.primary_language && (
                  <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                    <Language color="action" />
                    <Typography variant="body1">
                      Primary Language: {user.primary_language}
                    </Typography>
                  </Box>
                )}

                {user.additional_languages && user.additional_languages.length > 0 && (
                  <Box>
                    <Typography variant="body2" color="text.secondary" sx={{ mb: 1 }}>
                      Additional Languages:
                    </Typography>
                    <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1 }}>
                      {user.additional_languages.map((lang, index) => (
                        <Chip
                          key={index}
                          label={lang}
                          size="small"
                          variant="outlined"
                        />
                      ))}
                    </Box>
                  </Box>
                )}
              </Box>
            </CardContent>
          </Card>
        </Grid>

        {/* Platform Info Card */}
        <Grid item xs={12} md={6}>
          <Card elevation={3}>
            <CardContent>
              <Typography variant="h5" component="h2" gutterBottom>
                Platform Features
              </Typography>
              
              <Typography variant="body1" paragraph>
                Welcome to Sohbet, the Turkish academic social platform! This is currently version 0.2.0-academic.
              </Typography>

              <Typography variant="h6" gutterBottom>
                Available Features:
              </Typography>
              
              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                <Chip
                  label="✅ User Registration"
                  variant="outlined"
                  color="success"
                  size="small"
                />
                <Chip
                  label="✅ User Authentication"
                  variant="outlined"
                  color="success"
                  size="small"
                />
                <Chip
                  label="✅ Academic Profiles"
                  variant="outlined"
                  color="success"
                  size="small"
                />
                <Chip
                  label="✅ SQLite Persistence"
                  variant="outlined"
                  color="success"
                  size="small"
                />
                <Chip
                  label="✅ Secure Password Hashing"
                  variant="outlined"
                  color="success"
                  size="small"
                />
              </Box>

              <Typography variant="h6" sx={{ mt: 3 }} gutterBottom>
                Coming Soon:
              </Typography>
              
              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                <Chip
                  label="🔜 Course & Study Groups"
                  variant="outlined"
                  color="default"
                  size="small"
                />
                <Chip
                  label="🔜 Academic Networking"
                  variant="outlined"
                  color="default"
                  size="small"
                />
                <Chip
                  label="🔜 Q&A System"
                  variant="outlined"
                  color="default"
                  size="small"
                />
                <Chip
                  label="🔜 Real-time Messaging"
                  variant="outlined"
                  color="default"
                  size="small"
                />
              </Box>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Dashboard;