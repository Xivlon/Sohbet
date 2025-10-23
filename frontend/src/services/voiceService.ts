/**
 * Voice Service Client for Sohbet
 * 
 * This service provides an interface for interacting with voice/audio features
 * including creating and joining voice channels.
 */

export interface VoiceChannel {
  channel_id: number;
  name: string;
  description: string;
  creator_id: number;
  murmur_channel_id: number;
  max_users: number;
  is_temporary: boolean;
  created_at: string;
  active_users?: number;
}

export interface VoiceConnectionToken {
  connection_token: string;
  murmur_host: string;
  murmur_port: number;
  expires_at: string;
}

export interface CreateChannelRequest {
  name: string;
  description?: string;
  max_users?: number;
  is_temporary?: boolean;
}

export interface JoinChannelRequest {
  channel_id: number;
}

/**
 * Voice Service class for managing voice channels
 */
export class VoiceService {
  private baseUrl: string;
  private token: string | null;

  constructor(baseUrl: string = '/api') {
    this.baseUrl = baseUrl;
    this.token = null;
  }

  /**
   * Set the authentication token for API requests
   */
  setAuthToken(token: string): void {
    this.token = token;
  }

  /**
   * Get headers for API requests
   */
  private getHeaders(): HeadersInit {
    const headers: HeadersInit = {
      'Content-Type': 'application/json',
    };

    if (this.token) {
      headers['Authorization'] = `Bearer ${this.token}`;
    }

    return headers;
  }

  /**
   * Create a new voice channel
   */
  async createChannel(request: CreateChannelRequest): Promise<VoiceChannel> {
    const response = await fetch(`${this.baseUrl}/voice/channels`, {
      method: 'POST',
      headers: this.getHeaders(),
      body: JSON.stringify(request),
    });

    if (!response.ok) {
      throw new Error(`Failed to create channel: ${response.statusText}`);
    }

    return response.json();
  }

  /**
   * List all available voice channels
   */
  async listChannels(): Promise<VoiceChannel[]> {
    const response = await fetch(`${this.baseUrl}/voice/channels`, {
      method: 'GET',
      headers: this.getHeaders(),
    });

    if (!response.ok) {
      throw new Error(`Failed to list channels: ${response.statusText}`);
    }

    const data = await response.json();
    return data.channels || [];
  }

  /**
   * Get a specific voice channel by ID
   */
  async getChannel(channelId: number): Promise<VoiceChannel> {
    const response = await fetch(`${this.baseUrl}/voice/channels/${channelId}`, {
      method: 'GET',
      headers: this.getHeaders(),
    });

    if (!response.ok) {
      throw new Error(`Failed to get channel: ${response.statusText}`);
    }

    return response.json();
  }

  /**
   * Join a voice channel and get connection token
   */
  async joinChannel(request: JoinChannelRequest): Promise<VoiceConnectionToken> {
    const response = await fetch(`${this.baseUrl}/voice/join`, {
      method: 'POST',
      headers: this.getHeaders(),
      body: JSON.stringify(request),
    });

    if (!response.ok) {
      throw new Error(`Failed to join channel: ${response.statusText}`);
    }

    return response.json();
  }

  /**
   * Delete a voice channel
   */
  async deleteChannel(channelId: number): Promise<void> {
    const response = await fetch(`${this.baseUrl}/voice/channels/${channelId}`, {
      method: 'DELETE',
      headers: this.getHeaders(),
    });

    if (!response.ok) {
      throw new Error(`Failed to delete channel: ${response.statusText}`);
    }
  }

  /**
   * Check if voice service is enabled
   */
  async isEnabled(): Promise<boolean> {
    try {
      const response = await fetch(`${this.baseUrl}/voice/status`, {
        method: 'GET',
        headers: this.getHeaders(),
      });

      if (!response.ok) {
        return false;
      }

      const data = await response.json();
      return data.enabled === true;
    } catch (error) {
      return false;
    }
  }
}

// Export a singleton instance
export const voiceService = new VoiceService();
