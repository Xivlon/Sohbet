const express = require('express');
const cors = require('cors');
const { spawn } = require('child_process');
const path = require('path');

const app = express();
const PORT = 8080;

// Middleware
app.use(cors());
app.use(express.json());

// In-memory simulation of C++ backend responses
// This simulates what the C++ backend would return

// Server status endpoint
app.get('/api/status', (req, res) => {
  res.json({
    status: "ok",
    version: "0.2.0-academic",
    features: ["user_registration", "sqlite_persistence", "bcrypt_hashing", "react_frontend"]
  });
});

// Demo user endpoint
app.get('/api/users/demo', (req, res) => {
  res.json({
    id: 999,
    username: "demo_student",
    email: "demo@example.edu",
    university: "Demo University",
    department: "Computer Science",
    enrollment_year: 2023,
    primary_language: "Turkish"
  });
});

// Simple in-memory user storage for demo
const users = [
  {
    id: 1,
    username: "demo_student",
    email: "demo@example.edu",
    password_hash: "demo123", // In real app, this would be properly hashed
    university: "Demo University",
    department: "Computer Science",
    enrollment_year: 2023,
    primary_language: "Turkish"
  }
];

// User registration endpoint
app.post('/api/users', (req, res) => {
  const { username, email, password, university, department, enrollment_year, primary_language, additional_languages } = req.body;
  
  // Check if user already exists
  const existingUser = users.find(u => u.username === username || u.email === email);
  if (existingUser) {
    return res.status(409).json({ error: "Username or email already exists" });
  }
  
  // Create new user
  const newUser = {
    id: users.length + 1,
    username,
    email,
    password_hash: password, // In real app, hash this with bcrypt
    university,
    department,
    enrollment_year,
    primary_language,
    additional_languages
  };
  
  users.push(newUser);
  
  // Return user without password
  const { password_hash, ...userResponse } = newUser;
  res.status(201).json(userResponse);
});

// User login endpoint
app.post('/api/login', (req, res) => {
  const { username, password } = req.body;
  
  // Find user
  const user = users.find(u => u.username === username);
  if (!user) {
    return res.status(401).json({ error: "Invalid credentials" });
  }
  
  // Check password (in real app, use bcrypt.compare)
  if (user.password_hash !== password) {
    return res.status(401).json({ error: "Invalid credentials" });
  }
  
  // Generate mock JWT token
  const token = `mock_jwt_token_${Date.now()}_${user.id}`;
  
  // Return token and user (without password)
  const { password_hash, ...userResponse } = user;
  res.json({
    token,
    user: userResponse
  });
});

// 404 handler
app.use('*', (req, res) => {
  res.status(404).json({ error: "Endpoint not found" });
});

// Start server
app.listen(PORT, () => {
  console.log(`🚀 Sohbet API Proxy Server running on http://localhost:${PORT}`);
  console.log(`📱 Frontend should be running on http://localhost:3000`);
  console.log(`🔧 Available API endpoints:`);
  console.log(`   GET  /api/status`);
  console.log(`   GET  /api/users/demo`);
  console.log(`   POST /api/users`);
  console.log(`   POST /api/login`);
  console.log(`\n🎯 This proxy simulates the C++ backend functionality for React frontend testing`);
});

module.exports = app;