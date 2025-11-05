"use client";

import { useState, useEffect, useRef } from 'react';
import { useTranslations } from 'next-intl';
import { Mic, MicOff, PhoneOff, Volume2, Settings, Plus, Video, VideoOff } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Badge } from './ui/badge';
import { Slider } from './ui/slider';
import { voiceService, VoiceChannel } from '@/app/lib/voice-service';
import { webrtcService, VoiceParticipant } from '@/app/lib/webrtc-service';
import { useAuth } from '../contexts/auth-context';

interface Participant {
  id: string;
  name: string;
  university: string;
  isMuted: boolean;
  isSpeaking: boolean;
  isModerator: boolean;
}

export function Khave() {
  const t = useTranslations('khave');
  const tCommon = useTranslations('common');
  const { user } = useAuth();
  const [channels, setChannels] = useState<VoiceChannel[]>([]);
  const [currentChannel, setCurrentChannel] = useState<VoiceChannel | null>(null);
  const [isMuted, setIsMuted] = useState(false);
  const [isVideoEnabled, setIsVideoEnabled] = useState(false);
  const [isConnected, setIsConnected] = useState(false);
  const [volume, setVolume] = useState([75]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [showCreateChannel, setShowCreateChannel] = useState(false);
  const [newChannelName, setNewChannelName] = useState('');
  const [participants, setParticipants] = useState<VoiceParticipant[]>([]);
  const localVideoRef = useRef<HTMLVideoElement>(null);
  const remoteAudioRefs = useRef<Map<number, HTMLAudioElement>>(new Map());

  // Load channels on mount
  useEffect(() => {
    loadChannels();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Setup WebRTC callbacks
  useEffect(() => {
    // Listen for participant updates
    webrtcService.onParticipantUpdate((updatedParticipants) => {
      setParticipants(updatedParticipants);
    });

    // Listen for remote streams
    webrtcService.onRemoteStream((userId, stream) => {
      // Create or update audio element for this user
      let audioElement = remoteAudioRefs.current.get(userId);
      if (!audioElement) {
        audioElement = new Audio();
        audioElement.autoplay = true;
        remoteAudioRefs.current.set(userId, audioElement);
      }
      audioElement.srcObject = stream;
    });
  }, []);

  // Update local video preview
  useEffect(() => {
    if (localVideoRef.current && isConnected) {
      const localStream = webrtcService.getLocalStream();
      if (localStream) {
        localVideoRef.current.srcObject = localStream;
      }
    }
  }, [isConnected]);

  // Cleanup: leave channel when component unmounts or user navigates away
  useEffect(() => {
    return () => {
      if (currentChannel && isConnected) {
        // Clean up WebRTC
        webrtcService.leaveChannel();

        // Clean up the session on backend
        voiceService.leaveChannel(currentChannel.id).catch(err => {
          console.error('Error leaving channel on unmount:', err);
        });
      }
    };
  }, [currentChannel, isConnected]);

  const loadChannels = async () => {
    setLoading(true);
    setError(null);
    
    try {
      const response = await voiceService.getChannels('public');
      if (response.data) {
        setChannels(response.data.channels || []);
      } else {
        setError(response.error || 'Failed to load channels');
      }
    } catch (err) {
      setError('Failed to load channels');
      console.error('Error loading channels:', err);
    } finally {
      setLoading(false);
    }
  };

  const createChannel = async () => {
    if (!newChannelName.trim()) {
      return;
    }

    try {
      const response = await voiceService.createChannel({
        name: newChannelName,
        channel_type: 'public',
      });

      if (response.data) {
        setShowCreateChannel(false);
        setNewChannelName('');
        loadChannels(); // Reload channels
      } else {
        setError(response.error || 'Failed to create channel');
      }
    } catch (err) {
      setError('Failed to create channel');
      console.error('Error creating channel:', err);
    }
  };

  const joinChannel = async (channel: VoiceChannel) => {
    if (!user) {
      setError('You must be logged in to join a voice channel');
      return;
    }

    try {
      // Join via REST API first
      const response = await voiceService.joinChannel(channel.id);
      if (response.data) {
        // Initialize WebRTC connection
        await webrtcService.joinChannel(channel.id, user.id, true); // true = audio only for now

        setCurrentChannel(channel);
        setIsConnected(true);
        setError(null);
        console.log('Joined channel with token:', response.data);
      } else {
        setError(response.error || 'Failed to join channel');
      }
    } catch (err: any) {
      setError(err.message || 'Failed to join channel');
      console.error('Error joining channel:', err);
    }
  };

  const leaveChannel = async () => {
    if (!currentChannel) return;

    try {
      // Leave WebRTC session first
      webrtcService.leaveChannel();

      // Then leave via REST API
      const response = await voiceService.leaveChannel(currentChannel.id);
      if (response.status === 200) {
        setCurrentChannel(null);
        setIsConnected(false);
        setIsMuted(false);
        setIsVideoEnabled(false);
        setParticipants([]);
        loadChannels(); // Reload to get updated user counts
      } else {
        setError(response.error || 'Failed to leave channel');
      }
    } catch (err) {
      setError('Failed to leave channel');
      console.error('Error leaving channel:', err);
    }
  };

  const toggleMute = () => {
    const newMutedState = webrtcService.toggleMute();
    setIsMuted(newMutedState);
  };

  const toggleVideo = () => {
    const newVideoState = webrtcService.toggleVideo();
    setIsVideoEnabled(newVideoState);
  };

  return (
    <div className="h-full overflow-y-auto bg-background">
      <div className="max-w-full mx-auto p-4 pb-20">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 mb-4 border-b border-border">
          <div className="flex items-center justify-between">
            <div>
              <h2 className="text-primary">Khave - {t('voiceChat')}</h2>
              <p className="text-muted-foreground text-sm">{t('activeRooms')}</p>
            </div>
            <Button onClick={() => setShowCreateChannel(!showCreateChannel)} size="sm">
              <Plus className="w-4 h-4 mr-2" />
              {t('createRoom')}
            </Button>
          </div>
        </div>

        {/* Create Channel Dialog */}
        {showCreateChannel && (
          <Card className="mb-6">
            <CardHeader>
              <h3>{t('createRoom')}</h3>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                <div>
                  <label className="text-sm text-muted-foreground">{t('roomName')}</label>
                  <input
                    type="text"
                    value={newChannelName}
                    onChange={(e) => setNewChannelName(e.target.value)}
                    className="w-full p-2 border rounded mt-1"
                    placeholder={t('enterRoomName')}
                  />
                </div>
                <div className="flex gap-2">
                  <Button onClick={createChannel} disabled={!newChannelName.trim()}>
                    {tCommon('create')}
                  </Button>
                  <Button variant="outline" onClick={() => {
                    setShowCreateChannel(false);
                    setNewChannelName('');
                  }}>
                    {tCommon('cancel')}
                  </Button>
                </div>
              </div>
            </CardContent>
          </Card>
        )}

        {/* Error Message */}
        {error && (
          <Card className="mb-6 border-red-500">
            <CardContent className="pt-4">
              <p className="text-red-500">{error}</p>
            </CardContent>
          </Card>
        )}

        {/* Current Channel (if connected) */}
        {currentChannel && isConnected && (
          <Card className="mb-6 border-primary">
            <CardHeader>
              <div className="flex items-center justify-between">
                <div>
                  <h3 className="flex items-center gap-2">
                    <div className="w-3 h-3 bg-green-500 rounded-full animate-pulse"></div>
                    {currentChannel.name}
                  </h3>
                  <p className="text-muted-foreground text-sm">
                    {currentChannel.active_users} {t('participants').toLowerCase()}
                  </p>
                </div>
                <Badge variant="secondary">
                  {currentChannel.active_users} {t('participants').toLowerCase()}
                </Badge>
              </div>
            </CardHeader>
            <CardContent>
              {/* Local Video Preview (if enabled) */}
              {isVideoEnabled && (
                <div className="mb-4">
                  <video
                    ref={localVideoRef}
                    autoPlay
                    muted
                    playsInline
                    className="w-full rounded-lg border"
                  />
                  <p className="text-xs text-muted-foreground text-center mt-1">Video</p>
                </div>
              )}

              {/* Participants */}
              <div className="grid grid-cols-1 sm:grid-cols-2 gap-3 mb-6">
                {/* Show current user */}
                {user && (
                  <div className="flex items-center gap-3 p-3 border rounded-lg bg-primary/10">
                    <div className="relative">
                      <div className={`w-10 h-10 rounded-full flex items-center justify-center ${
                        !isMuted && participants.find(p => p.userId === user.id)?.isSpeaking
                          ? 'bg-primary text-primary-foreground ring-2 ring-primary'
                          : 'bg-secondary text-secondary-foreground'
                      }`}>
                        <span className="text-sm">{user.username.substring(0, 2).toUpperCase()}</span>
                      </div>
                      {isMuted && (
                        <MicOff className="w-4 h-4 absolute -bottom-1 -right-1 bg-destructive text-destructive-foreground rounded-full p-0.5" />
                      )}
                      {!isMuted && (
                        <Mic className="w-4 h-4 absolute -bottom-1 -right-1 bg-green-500 text-white rounded-full p-0.5" />
                      )}
                    </div>
                    <div className="flex-1 min-w-0">
                      <div className="font-medium text-sm truncate">{user.username}</div>
                      <div className="text-xs text-muted-foreground">{user.university}</div>
                    </div>
                  </div>
                )}

                {/* Show other participants */}
                {participants.filter(p => p.userId !== user?.id).map((participant) => (
                  <div key={participant.userId} className="flex items-center gap-3 p-3 border rounded-lg">
                    <div className="relative">
                      <div className={`w-10 h-10 rounded-full flex items-center justify-center ${
                        participant.isSpeaking ? 'bg-primary text-primary-foreground ring-2 ring-primary' : 'bg-secondary text-secondary-foreground'
                      }`}>
                        <span className="text-sm">{participant.username.substring(0, 2).toUpperCase()}</span>
                      </div>
                      {participant.isMuted && (
                        <MicOff className="w-4 h-4 absolute -bottom-1 -right-1 bg-destructive text-destructive-foreground rounded-full p-0.5" />
                      )}
                      {!participant.isMuted && (
                        <Mic className="w-4 h-4 absolute -bottom-1 -right-1 bg-green-500 text-white rounded-full p-0.5" />
                      )}
                    </div>
                    <div className="flex-1 min-w-0">
                      <div className="font-medium text-sm truncate">{participant.username}</div>
                      <div className="text-xs text-muted-foreground">{participant.university}</div>
                    </div>
                  </div>
                ))}

                {participants.length === 0 && !user && (
                  <div className="col-span-2 text-center text-muted-foreground py-4">
                    {t('participants')}
                  </div>
                )}
              </div>

              {/* Controls */}
              <div className="flex flex-col gap-4 p-4 bg-muted rounded-lg">
                <div className="flex items-center justify-center gap-4">
                  <Button
                    variant={isMuted ? "destructive" : "secondary"}
                    size="lg"
                    onClick={toggleMute}
                    className="w-12 h-12 rounded-full"
                    title={isMuted ? t('unmute') : t('mute')}
                  >
                    {isMuted ? <MicOff className="w-5 h-5" /> : <Mic className="w-5 h-5" />}
                  </Button>

                  <Button
                    variant={isVideoEnabled ? "secondary" : "outline"}
                    size="lg"
                    onClick={toggleVideo}
                    className="w-12 h-12 rounded-full"
                    title={isVideoEnabled ? t('stopSharing') : t('shareScreen')}
                  >
                    {isVideoEnabled ? <Video className="w-5 h-5" /> : <VideoOff className="w-5 h-5" />}
                  </Button>

                  <Button variant="secondary" size="lg" className="w-12 h-12 rounded-full" title={t('roomSettings')}>
                    <Settings className="w-5 h-5" />
                  </Button>

                  <Button
                    variant="destructive"
                    size="lg"
                    onClick={leaveChannel}
                    className="w-12 h-12 rounded-full"
                    title={t('leaveRoom')}
                  >
                    <PhoneOff className="w-5 h-5" />
                  </Button>
                </div>
                
                <div className="flex items-center gap-2 justify-center">
                  <Volume2 className="w-4 h-4" />
                  <div className="w-32">
                    <Slider
                      value={volume}
                      onValueChange={setVolume}
                      max={100}
                      step={1}
                      className="w-full"
                    />
                  </div>
                  <span className="text-sm text-muted-foreground w-8">{volume[0]}%</span>
                </div>
              </div>
            </CardContent>
          </Card>
        )}

        {/* Channel List */}
        <div className="space-y-4">
          {loading ? (
            <div className="text-center py-8 text-muted-foreground">{tCommon('loading')}...</div>
          ) : channels.length === 0 ? (
            <Card>
              <CardContent className="pt-6 text-center text-muted-foreground">
                <p>{t('createRoom')}</p>
              </CardContent>
            </Card>
          ) : (
            <div className="grid grid-cols-1 gap-4">
              {channels.map((channel) => (
                <Card key={channel.id} className="cursor-pointer hover:bg-accent transition-colors">
                  <CardHeader className="pb-3">
                    <div className="flex items-center justify-between">
                      <div className="flex items-center gap-2">
                        <div className="w-3 h-3 rounded-full bg-green-500"></div>
                        <h3 className="font-medium text-sm">{channel.name}</h3>
                      </div>
                      <Badge variant="outline">{channel.channel_type}</Badge>
                    </div>
                  </CardHeader>
                  <CardContent className="pt-0">
                    <div className="space-y-2">
                      <div className="flex items-center justify-between text-sm">
                        <span className="text-muted-foreground">{t('participants')}:</span>
                        <span>{channel.active_users} {t('participants').toLowerCase()}</span>
                      </div>
                    </div>
                    <Button
                      className="w-full mt-4"
                      disabled={currentChannel?.id === channel.id}
                      onClick={() => joinChannel(channel)}
                    >
                      {currentChannel?.id === channel.id ? t('leaveRoom') : t('joinRoom')}
                    </Button>
                  </CardContent>
                </Card>
              ))}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
