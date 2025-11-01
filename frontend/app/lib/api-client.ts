'use client';

import { API_URL } from './config';
import { isDebugEnabled } from './debug';

const API_BASE_URL = (API_URL || 'http://localhost:8080').replace(/\/+$/, ''); // remove trailing slashes

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
  role?: string;
  avatar_url?: string;
  banner_url?: string;
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

export interface Media {
  id: number;
  user_id: number;
  media_type: string;
  storage_key: string;
  file_name?: string;
  file_size?: number;
  mime_type?: string;
  url?: string;
  created_at?: string;
}

class ApiClient {
  private baseUrl: string;
  private token: string | null = null;
  private instanceId: string;
  private debug: boolean;

  constructor(baseUrl: string = API_BASE_URL) {
    // Use crypto.randomUUID() if available, otherwise fallback to Math.random()
    this.instanceId = typeof crypto !== 'undefined' && crypto.randomUUID 
      ? crypto.randomUUID().split('-')[0]
      : Math.random().toString(36).substring(7);
    this.baseUrl = baseUrl;
    // Enable debug logging in development or when explicitly enabled
    this.debug = isDebugEnabled();
    
    if (this.debug) {
      console.log('[API Client] Constructor called, instance ID:', this.instanceId, 'baseUrl:', baseUrl);
    }
    if (typeof window !== 'undefined') {
      this.token = localStorage.getItem('auth_token');
      if (this.debug) {
        console.log('[API Client] Token loaded from localStorage:', this.token ? `${this.token.substring(0, 20)}...` : 'null');
      }
    } else if (this.debug) {
      console.log('[API Client] Constructor running on server (no window), token remains null');
    }
  }

  setToken(token: string | null) {
    if (this.debug) {
      console.log('[API Client]', this.instanceId, 'setToken called with:', token ? `${token.substring(0, 20)}...` : 'null');
    }
    this.token = token;
    if (typeof window !== 'undefined') {
      if (token) {
        localStorage.setItem('auth_token', token);
        if (this.debug) {
          console.log('[API Client]', this.instanceId, 'Token stored in localStorage');
        }
      } else {
        localStorage.removeItem('auth_token');
        localStorage.removeItem('auth_user');
        if (this.debug) {
          console.log('[API Client]', this.instanceId, 'Token removed from localStorage');
        }
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
    if (this.debug) {
      console.log('[API Client]', this.instanceId, 'getToken called, returning:', this.token ? `${this.token.substring(0, 20)}...` : 'null');
    }
    return this.token;
  }

  /**
   * Synchronizes token from localStorage to instance variable.
   * This ensures all instances use the same token even if they're created separately.
   * @returns The current token from localStorage or instance variable
   */
  private syncTokenFromStorage(): string | null {
    let currentToken = this.token;
    if (typeof window !== 'undefined') {
      const storedToken = localStorage.getItem('auth_token');
      if (storedToken && storedToken !== this.token) {
        if (this.debug) {
          console.log('[API Client]', this.instanceId, 'Token mismatch! Instance token:', this.token ? 'exists' : 'null', ', localStorage token:', storedToken ? 'exists' : 'null');
        }
        currentToken = storedToken;
        this.token = storedToken; // Sync instance variable
      }
    }
    return currentToken;
  }

  private async request<T>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<ApiResponse<T>> {
    // Validate that API URL is configured
    if (!this.baseUrl) {
      console.error('API base URL not configured. Set NEXT_PUBLIC_API_URL environment variable.');
      return {
        error: 'API base URL not configured',
        status: 0,
      };
    }
    
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      ...options.headers as Record<string, string>,
    };

    // Always read token from localStorage to ensure we have the latest
    const currentToken = this.syncTokenFromStorage();

    if (currentToken) {
      headers['Authorization'] = `Bearer ${currentToken}`;
      if (this.debug) {
        console.log('[API Client]', this.instanceId, 'Including Authorization header with token:', currentToken.substring(0, 20) + '...');
      }
    } else if (this.debug) {
      console.log('[API Client]', this.instanceId, 'No token available, Authorization header NOT included');
    }

    try {
      const fullUrl = `${this.baseUrl}${endpoint}`;
      console.log('API Request:', fullUrl);
      
      const response = await fetch(fullUrl, {
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
      console.error('API Request Error:', error);
      console.error('Endpoint:', `${this.baseUrl}${endpoint}`);
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
    if (this.debug) {
      console.log('[API Client]', this.instanceId, 'Login attempt starting');
    }
    const response = await this.request<LoginResponse>('/api/login', {
      method: 'POST',
      body: JSON.stringify(data),
    });

    if (this.debug) {
      console.log('[API Client]', this.instanceId, 'Login response received:', {
        hasError: !!response.error,
        hasData: !!response.data,
        hasToken: !!response.data?.token,
        tokenPreview: response.data?.token ? response.data.token.substring(0, 20) + '...' : 'none'
      });
    }

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

  async uploadMedia(
    file: File,
    userId: number,
    mediaType: string
  ): Promise<ApiResponse<Media>> {
    const formData = new FormData();
    formData.append('file', file);
    formData.append('user_id', userId.toString());
    formData.append('media_type', mediaType);

    // Get current token using the same sync logic as other requests
    const currentToken = this.syncTokenFromStorage();

    try {
      const response = await fetch(`${this.baseUrl}/api/media/upload`, {
        method: 'POST',
        headers: currentToken ? { 'Authorization': `Bearer ${currentToken}` } : {},
        body: formData,
      });

      const data = await response.json();

      if (!response.ok) {
        return {
          error: data.error || 'Upload failed',
          status: response.status,
        };
      }

      return {
        data,
        status: response.status,
      };
    } catch (error) {
      console.error('Upload Error:', error);
      return {
        error: error instanceof Error ? error.message : 'Network error',
        status: 0,
      };
    }
  }

  async getUserMedia(userId: number): Promise<ApiResponse<Media[]>> {
    return this.request(`/api/users/${userId}/media`);
  }

  getMediaUrl(storageKey: string): string {
    return `${this.baseUrl}/api/media/file/${storageKey}`;
  }

  // Posts API
  async getPosts(limit: number = 50, offset: number = 0): Promise<ApiResponse<{ posts: any[]; total: number }>> {
    return this.request(`/api/posts?limit=${limit}&offset=${offset}`);
  }

  async createPost(content: string, visibility: string = 'friends'): Promise<ApiResponse<any>> {
    return this.request('/api/posts', {
      method: 'POST',
      body: JSON.stringify({ content, visibility }),
    });
  }

  async reactToPost(postId: number, reactionType: string = 'like'): Promise<ApiResponse<any>> {
    return this.request(`/api/posts/${postId}/react`, {
      method: 'POST',
      body: JSON.stringify({ reaction_type: reactionType }),
    });
  }

  async removeReaction(postId: number): Promise<ApiResponse<any>> {
    return this.request(`/api/posts/${postId}/react`, {
      method: 'DELETE',
    });
  }

  // Groups API
  async getGroups(limit: number = 50, offset: number = 0): Promise<ApiResponse<{ groups: any[]; total: number }>> {
    return this.request(`/api/groups?limit=${limit}&offset=${offset}`);
  }

  async createGroup(name: string, description: string): Promise<ApiResponse<any>> {
    return this.request('/api/groups', {
      method: 'POST',
      body: JSON.stringify({ name, description }),
    });
  }

  async joinGroup(groupId: number, userId: number): Promise<ApiResponse<any>> {
    return this.request(`/api/groups/${groupId}/members`, {
      method: 'POST',
      body: JSON.stringify({ user_id: userId, role: 'member' }),
    });
  }

  async leaveGroup(groupId: number, userId: number): Promise<ApiResponse<any>> {
    return this.request(`/api/groups/${groupId}/members/${userId}`, {
      method: 'DELETE',
    });
  }

  // Organizations API
  async getOrganizations(limit: number = 50, offset: number = 0): Promise<ApiResponse<{ organizations: any[]; total: number }>> {
    return this.request(`/api/organizations?limit=${limit}&offset=${offset}`);
  }

  async createOrganization(name: string, description: string, category: string): Promise<ApiResponse<any>> {
    return this.request('/api/organizations', {
      method: 'POST',
      body: JSON.stringify({ name, description, category }),
    });
  }

  // Users API
  async getUserById(userId: number): Promise<ApiResponse<any>> {
    return this.request(`/api/users/${userId}`);
  }

  // Conversations/Chat API
  async getConversations(): Promise<ApiResponse<{ conversations: any[]; total: number }>> {
    return this.request('/api/conversations');
  }

  async createConversation(otherUserId: number): Promise<ApiResponse<any>> {
    return this.request('/api/conversations', {
      method: 'POST',
      body: JSON.stringify({ other_user_id: otherUserId }),
    });
  }

  async getMessages(conversationId: number, limit: number = 50, offset: number = 0): Promise<ApiResponse<{ messages: any[] }>> {
    return this.request(`/api/conversations/${conversationId}/messages?limit=${limit}&offset=${offset}`);
  }

  async sendMessage(conversationId: number, content: string): Promise<ApiResponse<any>> {
    return this.request(`/api/conversations/${conversationId}/messages`, {
      method: 'POST',
      body: JSON.stringify({ content }),
    });
  }
}

export const apiClient = new ApiClient();
