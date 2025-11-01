import { apiClient } from './api-client';

export type {
  VoiceChannel,
  VoiceConnectionToken,
  CreateVoiceChannelData,
  VoiceChannelsResponse,
  ApiResponse,
} from './api-client';

/**
 * Voice Service - A wrapper around apiClient for voice channel operations.
 * This service uses the centralized apiClient which properly handles JWT authentication.
 */
class VoiceService {
  /**
   * Create a new voice channel
   */
  async createChannel(data: import('./api-client').CreateVoiceChannelData) {
    return apiClient.createVoiceChannel(data);
  }

  /**
   * Get all voice channels (optionally filtered by type)
   */
  async getChannels(channelType?: string) {
    return apiClient.getVoiceChannels(channelType);
  }

  /**
   * Get a specific voice channel by ID
   */
  async getChannel(channelId: number) {
    return apiClient.getVoiceChannel(channelId);
  }

  /**
   * Join a voice channel and get connection token
   */
  async joinChannel(channelId: number) {
    return apiClient.joinVoiceChannel(channelId);
  }

  /**
   * Leave a voice channel
   */
  async leaveChannel(channelId: number) {
    return apiClient.leaveVoiceChannel(channelId);
  }

  /**
   * Delete a voice channel (creator only)
   */
  async deleteChannel(channelId: number) {
    return apiClient.deleteVoiceChannel(channelId);
  }
}

export const voiceService = new VoiceService();
