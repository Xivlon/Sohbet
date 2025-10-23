/**
 * React hooks for voice service integration
 */

import { useState, useEffect, useCallback } from 'react';
import { voiceService, VoiceChannel, VoiceConnectionToken, CreateChannelRequest } from '../services/voiceService';

/**
 * Hook for checking if voice service is available
 */
export const useVoiceServiceStatus = () => {
  const [isEnabled, setIsEnabled] = useState<boolean>(false);
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    const checkStatus = async () => {
      try {
        const enabled = await voiceService.isEnabled();
        setIsEnabled(enabled);
      } catch (error) {
        console.error('Failed to check voice service status:', error);
        setIsEnabled(false);
      } finally {
        setLoading(false);
      }
    };

    checkStatus();
  }, []);

  return { isEnabled, loading };
};

/**
 * Hook for managing voice channels
 */
export const useVoiceChannels = () => {
  const [channels, setChannels] = useState<VoiceChannel[]>([]);
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);

  const loadChannels = useCallback(async () => {
    setLoading(true);
    setError(null);

    try {
      const channelList = await voiceService.listChannels();
      setChannels(channelList);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load channels');
    } finally {
      setLoading(false);
    }
  }, []);

  const createChannel = useCallback(async (request: CreateChannelRequest): Promise<VoiceChannel | null> => {
    setLoading(true);
    setError(null);

    try {
      const newChannel = await voiceService.createChannel(request);
      await loadChannels(); // Reload the channel list
      return newChannel;
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to create channel');
      return null;
    } finally {
      setLoading(false);
    }
  }, [loadChannels]);

  const deleteChannel = useCallback(async (channelId: number): Promise<boolean> => {
    setLoading(true);
    setError(null);

    try {
      await voiceService.deleteChannel(channelId);
      await loadChannels(); // Reload the channel list
      return true;
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to delete channel');
      return false;
    } finally {
      setLoading(false);
    }
  }, [loadChannels]);

  useEffect(() => {
    loadChannels();
  }, [loadChannels]);

  return {
    channels,
    loading,
    error,
    loadChannels,
    createChannel,
    deleteChannel,
  };
};

/**
 * Hook for joining a voice channel
 */
export const useJoinVoiceChannel = () => {
  const [connectionToken, setConnectionToken] = useState<VoiceConnectionToken | null>(null);
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);

  const joinChannel = useCallback(async (channelId: number) => {
    setLoading(true);
    setError(null);
    setConnectionToken(null);

    try {
      const token = await voiceService.joinChannel({ channel_id: channelId });
      setConnectionToken(token);
      return token;
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to join channel');
      return null;
    } finally {
      setLoading(false);
    }
  }, []);

  const clearToken = useCallback(() => {
    setConnectionToken(null);
    setError(null);
  }, []);

  return {
    connectionToken,
    loading,
    error,
    joinChannel,
    clearToken,
  };
};
