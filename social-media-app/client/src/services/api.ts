import axios from 'axios';

const API_BASE_URL = process.env.REACT_APP_API_BASE_URL || 'http://localhost:5000';

const apiClient = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

// Function to get posts
export const getPosts = async () => {
  const response = await apiClient.get('/posts');
  return response.data;
};

// Function to create a new post
export const createPost = async (postData) => {
  const response = await apiClient.post('/posts', postData);
  return response.data;
};

// Function to get a user profile
export const getUserProfile = async (userId) => {
  const response = await apiClient.get(`/users/${userId}`);
  return response.data;
};

// Function to authenticate a user
export const authenticateUser = async (credentials) => {
  const response = await apiClient.post('/auth/login', credentials);
  return response.data;
};

// Function to register a new user
export const registerUser = async (userData) => {
  const response = await apiClient.post('/auth/register', userData);
  return response.data;
};