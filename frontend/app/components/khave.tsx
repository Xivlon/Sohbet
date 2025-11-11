"use client";

import { useState, useEffect, useRef } from 'react';
import { useTranslations } from 'next-intl';
import { Mic, MicOff, PhoneOff, Volume2, VolumeX, Settings, Plus, Video, VideoOff, UserPlus, Radio } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Badge } from './ui/badge';
import { Slider } from './ui/slider';
import { voiceService, VoiceChannel } from '@/app/lib/voice-service';
import { webrtcService, VoiceParticipant } from '@/app/lib/webrtc-service';
import { useAuth } from '../contexts/auth-context';
import { ErrorBoundary } from './error-boundary';
import { websocketService } from '@/app/lib/websocket-service';
import { RefreshCw } from 'lucide-react';

interface Participant {
  id: string;
  name: string;
  university: string;
  isMuted: boolean;
  isSpeaking: boolean;
  isModerator: boolean;
}

function KhaveContent() {
  const t = useTranslations('khave');
  const tCommon = useTranslations('common');
  const { user } = useAuth();
  const [channels, setChannels] = useState<VoiceChannel[]>([]);
  const [currentChannel, setCurrentChannel] = useState<VoiceChannel | null>(null);
  const [isMuted, setIsMuted] = useState(false);
  const [isDeafened, setIsDeafened] = useState(false);
  const [isVideoEnabled, setIsVideoEnabled] = useState(false);
  const [isConnected, setIsConnected] = useState(false);
  const [volume, setVolume] = useState([75]);
  const [showSettings, setShowSettings] = useState(false);
  const [showInvite, setShowInvite] = useState(false);
  const [connectionQuality, setConnectionQuality] = useState<'good' | 'medium' | 'poor'>('good');
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [showCreateChannel, setShowCreateChannel] = useState(false);
  const [newChannelName, setNewChannelName] = useState('');
  const [participants, setParticipants] = useState<VoiceParticipant[]>([]);
  const localVideoRef = useRef<HTMLVideoElement>(null);
  const announcementRef = useRef<HTMLDivElement>(null);
  const [announcement, setAnnouncement] = useState('');
  const createChannelInputRef = useRef<HTMLInputElement>(null);
  const invitePanelRef = useRef<HTMLDivElement>(null);
  const settingsPanelRef = useRef<HTMLDivElement>(null);
  const [isRefreshing, setIsRefreshing] = useState(false);

  // Load channels on mount
  useEffect(() => {
    loadChannels();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Auto-refresh channels periodically (every 30 seconds)
  useEffect(() => {
    const intervalId = setInterval(() => {
      if (!isConnected) {
        // Only auto-refresh if not in a channel
        loadChannels(true);
      }
    }, 30000); // 30 seconds

    return () => clearInterval(intervalId);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [isConnected]);

  // Listen for WebSocket events to refresh channel list
  useEffect(() => {
    // Refresh when users join/leave channels (affects active_users count)
    const handleUserJoined = () => {
      if (!isConnected) {
        loadChannels(true);
      }
    };

    const handleUserLeft = () => {
      if (!isConnected) {
        loadChannels(true);
      }
    };

    websocketService.on('voice:user-joined', handleUserJoined);
    websocketService.on('voice:user-left', handleUserLeft);

    return () => {
      websocketService.off('voice:user-joined', handleUserJoined);
      websocketService.off('voice:user-left', handleUserLeft);
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [isConnected]);

  // Setup WebRTC callbacks
  useEffect(() => {
    // Listen for participant updates
    webrtcService.onParticipantUpdate((updatedParticipants) => {
      const prevCount = participants.length;
      const newCount = updatedParticipants.length;

      // Announce participant changes for screen readers
      if (newCount > prevCount) {
        const newParticipant = updatedParticipants.find(
          p => !participants.some(prev => prev.userId === p.userId)
        );
        if (newParticipant) {
          setAnnouncement(`${newParticipant.username} joined the voice channel`);
        }
      } else if (newCount < prevCount) {
        const leftParticipant = participants.find(
          p => !updatedParticipants.some(updated => updated.userId === p.userId)
        );
        if (leftParticipant) {
          setAnnouncement(`${leftParticipant.username} left the voice channel`);
        }
      }

      setParticipants(updatedParticipants);
    });

    // Listen for remote streams - no need to create audio elements
    // as webrtc-service.ts handles audio routing via Web Audio API
    webrtcService.onRemoteStream((userId, stream) => {
      console.log(`Received remote stream from user ${userId}`);
    });

    // Listen for connection quality updates
    webrtcService.onConnectionQuality((quality) => {
      setConnectionQuality(quality);
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

  const loadChannels = async (showRefreshingIndicator = false) => {
    if (showRefreshingIndicator) {
      setIsRefreshing(true);
    } else {
      setLoading(true);
    }
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
      setIsRefreshing(false);
    }
  };

  const handleRefreshChannels = async () => {
    await loadChannels(true);
    setAnnouncement('Channel list refreshed');
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
        // Initialize WebRTC connection (audio only initially)
        await webrtcService.joinChannel(channel.id, user.id, true); // true = audio only

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

  const toggleDeafen = () => {
    const newDeafenedState = webrtcService.toggleDeafen();
    setIsDeafened(newDeafenedState);
    // If deafened, mute the user as well
    if (newDeafenedState) {
      setIsMuted(true);
    }
  };

  const toggleVideo = async () => {
    try {
      const newVideoState = await webrtcService.toggleVideo();
      setIsVideoEnabled(newVideoState);
    } catch (err: any) {
      setError(err.message || 'Failed to toggle video');
      console.error('Error toggling video:', err);
    }
  };

  const setParticipantVolume = (userId: number, volume: number) => {
    webrtcService.setParticipantVolume(userId, volume / 100);
  };

  // Focus management for modals
  useEffect(() => {
    if (showCreateChannel && createChannelInputRef.current) {
      createChannelInputRef.current.focus();
    }
  }, [showCreateChannel]);

  useEffect(() => {
    if (showInvite && invitePanelRef.current) {
      // Focus first focusable element in invite panel
      const focusableElement = invitePanelRef.current.querySelector<HTMLElement>(
        'button, input, [tabindex]:not([tabindex="-1"])'
      );
      focusableElement?.focus();
    }
  }, [showInvite]);

  useEffect(() => {
    if (showSettings && settingsPanelRef.current) {
      // Focus settings panel
      settingsPanelRef.current.focus();
    }
  }, [showSettings]);

  return (
    <div className="h-full overflow-y-auto bg-background">
      {/* Screen reader announcements for participant events */}
      <div
        ref={announcementRef}
        role="status"
        aria-live="polite"
        aria-atomic="true"
        className="sr-only"
      >
        {announcement}
      </div>
      <div className="max-w-full mx-auto p-4 pb-20">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 mb-4 border-b border-border">
          <div className="flex items-center justify-between">
            <div>
              <h2 className="text-primary">Khave - {t('voiceChat')}</h2>
              <p className="text-muted-foreground text-sm">{t('activeRooms')}</p>
            </div>
            <div className="flex gap-2">
              <Button
                onClick={handleRefreshChannels}
                size="sm"
                variant="outline"
                disabled={isRefreshing}
                aria-label="Refresh channel list"
                title="Refresh channel list"
              >
                <RefreshCw className={`w-4 h-4 ${isRefreshing ? 'animate-spin' : ''}`} aria-hidden="true" />
              </Button>
              <Button
                onClick={() => setShowCreateChannel(!showCreateChannel)}
                size="sm"
                aria-label={t('createRoom')}
                aria-expanded={showCreateChannel}
              >
                <Plus className="w-4 h-4 mr-2" aria-hidden="true" />
                {t('createRoom')}
              </Button>
            </div>
          </div>
        </div>

        {/* Create Channel Dialog */}
        {showCreateChannel && (
          <Card className="mb-6" role="dialog" aria-labelledby="create-channel-title">
            <CardHeader>
              <h3 id="create-channel-title">{t('createRoom')}</h3>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                <div>
                  <label htmlFor="channel-name-input" className="text-sm text-muted-foreground">
                    {t('roomName')}
                  </label>
                  <input
                    ref={createChannelInputRef}
                    id="channel-name-input"
                    type="text"
                    value={newChannelName}
                    onChange={(e) => setNewChannelName(e.target.value)}
                    onKeyDown={(e) => {
                      if (e.key === 'Enter' && newChannelName.trim()) {
                        createChannel();
                      } else if (e.key === 'Escape') {
                        setShowCreateChannel(false);
                        setNewChannelName('');
                      }
                    }}
                    className="w-full p-2 border rounded mt-1"
                    placeholder={t('enterRoomName')}
                    aria-required="true"
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
                  <p className="text-muted-foreground text-sm flex items-center gap-2">
                    <Radio className={`w-3 h-3 ${
                      connectionQuality === 'good' ? 'text-green-500' :
                      connectionQuality === 'medium' ? 'text-yellow-500' :
                      'text-red-500'
                    }`} />
                    {currentChannel.active_users} {t('participants').toLowerCase()}
                  </p>
                </div>
                <div className="flex items-center gap-2">
                  <Badge variant="secondary">
                    {currentChannel.active_users} {t('participants').toLowerCase()}
                  </Badge>
                  <Button
                    variant="outline"
                    size="sm"
                    onClick={() => setShowInvite(!showInvite)}
                    aria-label="Invite to room"
                    aria-expanded={showInvite}
                  >
                    <UserPlus className="w-4 h-4" aria-hidden="true" />
                  </Button>
                </div>
              </div>
            </CardHeader>
            <CardContent>
              {/* Invite Panel */}
              {showInvite && (
                <div
                  ref={invitePanelRef}
                  className="mb-4 p-4 bg-accent rounded-lg"
                  role="region"
                  aria-labelledby="invite-panel-title"
                  tabIndex={-1}
                >
                  <h4 id="invite-panel-title" className="font-medium mb-2 flex items-center gap-2">
                    <UserPlus className="w-4 h-4" aria-hidden="true" />
                    {t('inviteToRoom')}
                  </h4>
                  <p className="text-sm text-muted-foreground mb-3">
                    Share this room with your friends:
                  </p>
                  <div className="flex gap-2">
                    <input
                      type="text"
                      readOnly
                      value={`${window.location.origin}/khave?room=${currentChannel.id}`}
                      className="flex-1 p-2 text-sm border rounded bg-background"
                      aria-label="Invitation link"
                      onFocus={(e) => e.target.select()}
                    />
                    <Button
                      size="sm"
                      onClick={() => {
                        navigator.clipboard.writeText(
                          `${window.location.origin}/khave?room=${currentChannel.id}`
                        );
                        setAnnouncement('Invitation link copied to clipboard');
                      }}
                      aria-label="Copy invitation link to clipboard"
                    >
                      {tCommon('copy')}
                    </Button>
                  </div>
                </div>
              )}

              {/* Room Settings Panel */}
              {showSettings && (
                <div
                  ref={settingsPanelRef}
                  className="mb-4 p-4 bg-accent rounded-lg"
                  role="region"
                  aria-labelledby="settings-panel-title"
                  tabIndex={-1}
                >
                  <h4 id="settings-panel-title" className="font-medium mb-3 flex items-center gap-2">
                    <Settings className="w-4 h-4" aria-hidden="true" />
                    {t('roomSettings')}
                  </h4>
                  <div className="space-y-3">
                    <div>
                      <label className="text-sm text-muted-foreground block mb-1">
                        Room Name
                      </label>
                      <p className="text-sm font-medium">{currentChannel.name}</p>
                    </div>
                    <div>
                      <label className="text-sm text-muted-foreground block mb-1">
                        Room Type
                      </label>
                      <Badge variant="outline">{currentChannel.channel_type}</Badge>
                    </div>
                    <div>
                      <label className="text-sm text-muted-foreground block mb-1">
                        Created At
                      </label>
                      <p className="text-sm">
                        {new Date(currentChannel.created_at).toLocaleDateString()}
                      </p>
                    </div>
                  </div>
                </div>
              )}

              {/* Local Video Preview (if enabled) */}
              {isVideoEnabled && (
                <div className="mb-4">
                  <video
                    ref={localVideoRef}
                    autoPlay
                    muted
                    playsInline
                    className="w-full rounded-lg border"
                    aria-label={`Video preview for ${user?.username || 'local user'}`}
                    role="img"
                  />
                  <p className="text-xs text-muted-foreground text-center mt-1">Video</p>
                </div>
              )}

              {/* Participants */}
              <div
                className="grid grid-cols-1 sm:grid-cols-2 gap-3 mb-6"
                role="list"
                aria-label="Voice channel participants"
              >
                {/* Show current user */}
                {user && (
                  <div
                    className="flex items-center gap-3 p-3 border rounded-lg bg-primary/10"
                    role="listitem"
                    tabIndex={0}
                    aria-label={`You: ${user.username}, ${isMuted ? 'muted' : 'unmuted'}`}
                  >
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
                  <div
                    key={participant.userId}
                    className="flex flex-col gap-2 p-3 border rounded-lg focus:ring-2 focus:ring-primary focus:outline-none"
                    role="listitem"
                    tabIndex={0}
                    aria-label={`${participant.username}, ${participant.isMuted ? 'muted' : 'unmuted'}, ${participant.isSpeaking ? 'speaking' : 'not speaking'}`}
                    onKeyDown={(e) => handleParticipantKeyDown(e, participant)}
                  >
                    <div className="flex items-center gap-3">
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
                    {/* Individual volume control */}
                    <div className="flex items-center gap-2 px-2">
                      <Volume2 className="w-3 h-3 text-muted-foreground" aria-hidden="true" />
                      <Slider
                        value={[webrtcService.getParticipantVolume(participant.userId) * 100]}
                        onValueChange={(value) => setParticipantVolume(participant.userId, value[0])}
                        max={100}
                        step={1}
                        className="flex-1"
                        aria-label={`Volume control for ${participant.username}`}
                      />
                      <span className="text-xs text-muted-foreground w-8 text-right" aria-live="polite">
                        {Math.round(webrtcService.getParticipantVolume(participant.userId) * 100)}%
                      </span>
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
              <div className="flex flex-col gap-4 p-4 bg-muted rounded-lg" role="group" aria-label="Voice channel controls">
                <div className="flex items-center justify-center gap-3 flex-wrap">
                  <Button
                    variant={isMuted ? "destructive" : "secondary"}
                    size="lg"
                    onClick={toggleMute}
                    className="w-12 h-12 rounded-full"
                    title={isMuted ? t('unmute') : t('mute')}
                    aria-label={isMuted ? t('unmute') : t('mute')}
                    aria-pressed={isMuted}
                    disabled={isDeafened}
                  >
                    {isMuted ? <MicOff className="w-5 h-5" aria-hidden="true" /> : <Mic className="w-5 h-5" aria-hidden="true" />}
                  </Button>

                  <Button
                    variant={isDeafened ? "destructive" : "secondary"}
                    size="lg"
                    onClick={toggleDeafen}
                    className="w-12 h-12 rounded-full"
                    title={isDeafened ? t('undeafen') : t('deafen')}
                    aria-label={isDeafened ? t('undeafen') : t('deafen')}
                    aria-pressed={isDeafened}
                  >
                    {isDeafened ? <VolumeX className="w-5 h-5" aria-hidden="true" /> : <Volume2 className="w-5 h-5" aria-hidden="true" />}
                  </Button>

                  <Button
                    variant={isVideoEnabled ? "secondary" : "outline"}
                    size="lg"
                    onClick={toggleVideo}
                    className="w-12 h-12 rounded-full"
                    title={isVideoEnabled ? t('stopSharing') : t('shareScreen')}
                    aria-label={isVideoEnabled ? 'Stop video' : 'Start video'}
                    aria-pressed={isVideoEnabled}
                  >
                    {isVideoEnabled ? <Video className="w-5 h-5" aria-hidden="true" /> : <VideoOff className="w-5 h-5" aria-hidden="true" />}
                  </Button>

                  <Button
                    variant="secondary"
                    size="lg"
                    className="w-12 h-12 rounded-full"
                    title={t('roomSettings')}
                    aria-label={t('roomSettings')}
                    aria-expanded={showSettings}
                    onClick={() => setShowSettings(!showSettings)}
                  >
                    <Settings className="w-5 h-5" aria-hidden="true" />
                  </Button>

                  <Button
                    variant="destructive"
                    size="lg"
                    onClick={leaveChannel}
                    className="w-12 h-12 rounded-full"
                    title={t('leaveRoom')}
                    aria-label={t('leaveRoom')}
                  >
                    <PhoneOff className="w-5 h-5" aria-hidden="true" />
                  </Button>
                </div>

                <div className="flex items-center gap-2 justify-center">
                  <Volume2 className="w-4 h-4" aria-hidden="true" />
                  <div className="w-32">
                    <Slider
                      value={volume}
                      onValueChange={setVolume}
                      max={100}
                      step={1}
                      className="w-full"
                      aria-label="Master volume control"
                    />
                  </div>
                  <span className="text-sm text-muted-foreground w-8" aria-live="polite">{volume[0]}%</span>
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

// Export Khave component wrapped with Error Boundary
export function Khave() {
  return (
    <ErrorBoundary
      onError={(error, errorInfo) => {
        // Log error to monitoring service if available
        console.error('Khave error:', error, errorInfo);
      }}
    >
      <KhaveContent />
    </ErrorBoundary>
  );
}
