const API_BASE_URL = process.env.NEXT_PUBLIC_API_URL || 'http://localhost:8080';

export interface ApiResponse<T> {
  data?: T;
  error?: string;
  status: number;
}

export interface User {
  id: number;
  username: string;
  email: string;
  name?: string;
  position?: string;
  phone_number?: string;
  university?: string;
  department?: string;
  enrollment_year?: number;
  primary_language?: string;
  additional_languages?: string[];
  created_at?: string;
  warnings?: number;
}

export interface RegisterData {
  username: string;
  email: string;
  password: string;
  university?: string;
  department?: string;
  enrollment_year?: number;
  primary_language?: string;
  additional_languages?: string[];
}

export interface LoginData {
  username: string;
  password: string;
}

export interface LoginResponse {
  token: string;
  user: User;
}

class ApiClient {
  private baseUrl: string;
  private token: string | null = null;

  constructor(baseUrl: string = API_BASE_URL) {
    this.baseUrl = baseUrl;
    if (typeof window !== 'undefined') {
      this.token = localStorage.getItem('auth_token');
    }
  }

  setToken(token: string | null) {
    this.token = token;
    if (typeof window !== 'undefined') {
      if (token) {
        localStorage.setItem('auth_token', token);
      } else {
        localStorage.removeItem('auth_token');
        localStorage.removeItem('auth_user');
      }
    }
  }

  setUser(user: User | null) {
    if (typeof window !== 'undefined') {
      if (user) {
        localStorage.setItem('auth_user', JSON.stringify(user));
      } else {
        localStorage.removeItem('auth_user');
      }
    }
  }

  getUser(): User | null {
    if (typeof window !== 'undefined') {
      const userStr = localStorage.getItem('auth_user');
      if (userStr) {
        try {
          return JSON.parse(userStr);
        } catch {
          return null;
        }
      }
    }
    return null;
  }

  getToken(): string | null {
    return this.token;
  }

  private async request<T>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<ApiResponse<T>> {
    const headers: HeadersInit = {
      'Content-Type': 'application/json',
      ...options.headers,
    };

    if (this.token) {
      headers['Authorization'] = `Bearer ${this.token}`;
    }

    try {
      const response = await fetch(`${this.baseUrl}${endpoint}`, {
        ...options,
        headers,
      });

      const data = await response.json();

      if (!response.ok) {
        return {
          error: data.error || 'Request failed',
          status: response.status,
        };
      }

      return {
        data,
        status: response.status,
      };
    } catch (error) {
      return {
        error: error instanceof Error ? error.message : 'Network error',
        status: 0,
      };
    }
  }

  async getStatus(): Promise<ApiResponse<{ status: string; version: string; features: string[] }>> {
    return this.request('/api/status');
  }

  async register(data: RegisterData): Promise<ApiResponse<User>> {
    return this.request('/api/users', {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  async login(data: LoginData): Promise<ApiResponse<LoginResponse>> {
    const response = await this.request<LoginResponse>('/api/login', {
      method: 'POST',
      body: JSON.stringify(data),
    });

    if (response.data?.token) {
      this.setToken(response.data.token);
      this.setUser(response.data.user);
    }

    return response;
  }

  logout() {
    this.setToken(null);
  }

  async getDemoUser(): Promise<ApiResponse<User>> {
    return this.request('/api/users/demo');
  }
}

export const apiClient = new ApiClient();
