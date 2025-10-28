const API_BASE_URL = process.env.NEXT_PUBLIC_API_URL || 'http://0.0.0.0:8080';

export interface VoiceChannel {
  id: number;
  name: string;
  channel_type: 'public' | 'group' | 'private';
  group_id?: number;
  organization_id?: number;
  active_users: number;
  created_at: string;
}

export interface VoiceConnectionToken {
  session_id: number;
  channel_id: number;
  connection_token: string;
  murmur_host: string;
  murmur_port: number;
  expires_at: string;
}

export interface CreateVoiceChannelData {
  name: string;
  channel_type?: 'public' | 'group' | 'private';
  group_id?: number;
  organization_id?: number;
}

export interface VoiceChannelsResponse {
  channels: VoiceChannel[];
  count: number;
}

export interface ApiResponse<T> {
  data?: T;
  error?: string;
  status: number;
}

class VoiceService {
  private baseUrl: string;

  constructor(baseUrl: string = API_BASE_URL) {
    this.baseUrl = baseUrl;
  }

  private getAuthHeaders(): HeadersInit {
    const token = typeof window !== 'undefined' ? localStorage.getItem('auth_token') : null;
    return {
      'Content-Type': 'application/json',
      ...(token && { 'Authorization': `Bearer ${token}` }),
    };
  }

  private async request<T>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<ApiResponse<T>> {
    const headers = {
      ...this.getAuthHeaders(),
      ...options.headers,
    };

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
      console.error('Voice Service Error:', error);
      return {
        error: error instanceof Error ? error.message : 'Network error',
        status: 0,
      };
    }
  }

  /**
   * Create a new voice channel
   */
  async createChannel(data: CreateVoiceChannelData): Promise<ApiResponse<VoiceChannel>> {
    return this.request('/api/voice/channels', {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  /**
   * Get all voice channels (optionally filtered by type)
   */
  async getChannels(channelType?: string): Promise<ApiResponse<VoiceChannelsResponse>> {
    const params = channelType ? `?channel_type=${channelType}` : '';
    return this.request(`/api/voice/channels${params}`);
  }

  /**
   * Get a specific voice channel by ID
   */
  async getChannel(channelId: number): Promise<ApiResponse<VoiceChannel>> {
    return this.request(`/api/voice/channels/${channelId}`);
  }

  /**
   * Join a voice channel and get connection token
   */
  async joinChannel(channelId: number): Promise<ApiResponse<VoiceConnectionToken>> {
    return this.request(`/api/voice/channels/${channelId}/join`, {
      method: 'POST',
    });
  }

  /**
   * Leave a voice channel
   */
  async leaveChannel(channelId: number): Promise<ApiResponse<{ message: string }>> {
    return this.request(`/api/voice/channels/${channelId}/leave`, {
      method: 'DELETE',
    });
  }

  /**
   * Delete a voice channel (creator only)
   */
  async deleteChannel(channelId: number): Promise<ApiResponse<{ message: string }>> {
    return this.request(`/api/voice/channels/${channelId}`, {
      method: 'DELETE',
    });
  }
}

export const voiceService = new VoiceService();
