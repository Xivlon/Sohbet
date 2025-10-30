'use client';

const API_BASE_URL = (process.env.NEXT_PUBLIC_API_URL || 'http://localhost:8080').replace(/\/+$/, ''); // remove trailing slashes

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

    if (this.token) {
      headers['Authorization'] = `Bearer ${this.token}`;
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

  async uploadMedia(
    file: File,
    userId: number,
    mediaType: string
  ): Promise<ApiResponse<Media>> {
    const formData = new FormData();
    formData.append('file', file);
    formData.append('user_id', userId.toString());
    formData.append('media_type', mediaType);

    try {
      const response = await fetch(`${this.baseUrl}/api/media/upload`, {
        method: 'POST',
        headers: this.token ? { 'Authorization': `Bearer ${this.token}` } : {},
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
