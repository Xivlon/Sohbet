import axios from 'axios';

const API_BASE_URL = '/api'; // Using proxy to backend

export interface User {
  id?: number;
  username: string;
  email: string;
  university?: string;
  department?: string;
  enrollment_year?: number;
  primary_language?: string;
  additional_languages?: string[];
}

export interface LoginRequest {
  username: string;
  password: string;
}

export interface RegisterRequest {
  username: string;
  email: string;
  password: string;
  university?: string;
  department?: string;
  enrollment_year?: number;
  primary_language?: string;
  additional_languages?: string[];
}

export interface LoginResponse {
  token: string;
  user: User;
}

export interface ServerStatus {
  status: string;
  version: string;
  features: string[];
}

class ApiService {
  private axiosInstance;

  constructor() {
    this.axiosInstance = axios.create({
      baseURL: API_BASE_URL,
      headers: {
        'Content-Type': 'application/json',
      },
    });
  }

  // Get server status
  async getStatus(): Promise<ServerStatus> {
    const response = await this.axiosInstance.get('/status');
    return response.data;
  }

  // Get demo user
  async getDemoUser(): Promise<User> {
    const response = await this.axiosInstance.get('/users/demo');
    return response.data;
  }

  // Register a new user
  async register(userData: RegisterRequest): Promise<User> {
    const response = await this.axiosInstance.post('/users', userData);
    return response.data;
  }

  // Login user
  async login(credentials: LoginRequest): Promise<LoginResponse> {
    const response = await this.axiosInstance.post('/login', credentials);
    return response.data;
  }

  // Set authentication token for future requests
  setAuthToken(token: string) {
    this.axiosInstance.defaults.headers.common['Authorization'] = `Bearer ${token}`;
  }

  // Remove authentication token
  removeAuthToken() {
    delete this.axiosInstance.defaults.headers.common['Authorization'];
  }
}

export default new ApiService();