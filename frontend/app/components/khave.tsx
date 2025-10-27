"use client";

import { useState, useEffect } from 'react';
import { Mic, MicOff, PhoneCall, PhoneOff, Users, Volume2, Settings, Plus } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from './ui/tabs';
import { Badge } from './ui/badge';
import { Slider } from './ui/slider';
import { voiceService, VoiceChannel } from '@/app/lib/voice-service';

interface Participant {
  id: string;
  name: string;
  university: string;
  isMuted: boolean;
  isSpeaking: boolean;
  isModerator: boolean;
}

const mockParticipants: Participant[] = [
  { id: '1', name: 'Ali Uzun', university: 'İTÜ', isMuted: false, isSpeaking: true, isModerator: false },
  { id: '2', name: 'Dr. Ahmet Yılmaz', university: 'İTÜ', isMuted: false, isSpeaking: false, isModerator: true },
  { id: '3', name: 'Zeynep Kaya', university: 'İTÜ', isMuted: true, isSpeaking: false, isModerator: false },
  { id: '4', name: 'Mehmet Demir', university: 'İTÜ', isMuted: false, isSpeaking: false, isModerator: false }
];

export function Khave() {
  const [activeTab, setActiveTab] = useState<'local' | 'global'>('local');
  const [channels, setChannels] = useState<VoiceChannel[]>([]);
  const [currentChannel, setCurrentChannel] = useState<VoiceChannel | null>(null);
  const [isMuted, setIsMuted] = useState(false);
  const [isConnected, setIsConnected] = useState(false);
  const [volume, setVolume] = useState([75]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [showCreateChannel, setShowCreateChannel] = useState(false);
  const [newChannelName, setNewChannelName] = useState('');

  // Load channels on mount and tab change
  useEffect(() => {
    loadChannels();
  }, [activeTab]);

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
    try {
      const response = await voiceService.joinChannel(channel.id);
      if (response.data) {
        setCurrentChannel(channel);
        setIsConnected(true);
        console.log('Joined channel with token:', response.data);
      } else {
        setError(response.error || 'Failed to join channel');
      }
    } catch (err) {
      setError('Failed to join channel');
      console.error('Error joining channel:', err);
    }
  };

  const leaveChannel = async () => {
    if (!currentChannel) return;

    try {
      const response = await voiceService.leaveChannel(currentChannel.id);
      if (response.status === 200) {
        setCurrentChannel(null);
        setIsConnected(false);
        setIsMuted(false);
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
    setIsMuted(!isMuted);
  };

  return (
    <div className="h-full overflow-y-auto bg-background">
      <div className="max-w-full mx-auto p-4 pb-20">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 mb-4 border-b border-border">
          <div className="flex items-center justify-between">
            <div>
              <h2 className="text-primary">Khave - Sesli Sohbet</h2>
              <p className="text-muted-foreground text-sm">Gerçek zamanlı sesli tartışma odaları</p>
            </div>
            <Button onClick={() => setShowCreateChannel(!showCreateChannel)} size="sm">
              <Plus className="w-4 h-4 mr-2" />
              Yeni Oda
            </Button>
          </div>
        </div>

        {/* Create Channel Dialog */}
        {showCreateChannel && (
          <Card className="mb-6">
            <CardHeader>
              <h3>Yeni Sesli Oda Oluştur</h3>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                <div>
                  <label className="text-sm text-muted-foreground">Oda Adı</label>
                  <input
                    type="text"
                    value={newChannelName}
                    onChange={(e) => setNewChannelName(e.target.value)}
                    className="w-full p-2 border rounded mt-1"
                    placeholder="Oda adını girin..."
                  />
                </div>
                <div className="flex gap-2">
                  <Button onClick={createChannel} disabled={!newChannelName.trim()}>
                    Oluştur
                  </Button>
                  <Button variant="outline" onClick={() => {
                    setShowCreateChannel(false);
                    setNewChannelName('');
                  }}>
                    İptal
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
                    {currentChannel.active_users} kullanıcı aktif
                  </p>
                </div>
                <Badge variant="secondary">
                  {currentChannel.active_users} kişi
                </Badge>
              </div>
            </CardHeader>
            <CardContent>
              {/* Participants */}
              <div className="grid grid-cols-1 sm:grid-cols-2 gap-3 mb-6">
                {mockParticipants.map((participant) => (
                  <div key={participant.id} className="flex items-center gap-3 p-3 border rounded-lg">
                    <div className="relative">
                      <div className={`w-10 h-10 rounded-full flex items-center justify-center ${
                        participant.isSpeaking ? 'bg-primary text-primary-foreground' : 'bg-secondary text-secondary-foreground'
                      }`}>
                        <span className="text-sm">{participant.name.split(' ').map(n => n[0]).join('')}</span>
                      </div>
                      {participant.isMuted && (
                        <MicOff className="w-4 h-4 absolute -bottom-1 -right-1 bg-destructive text-destructive-foreground rounded-full p-0.5" />
                      )}
                      {participant.isModerator && (
                        <div className="w-3 h-3 bg-primary rounded-full absolute -top-1 -right-1"></div>
                      )}
                    </div>
                    <div className="flex-1 min-w-0">
                      <div className="font-medium text-sm truncate">{participant.name}</div>
                      <div className="text-xs text-muted-foreground">{participant.university}</div>
                    </div>
                  </div>
                ))}
              </div>

              {/* Controls */}
              <div className="flex flex-col gap-4 p-4 bg-muted rounded-lg">
                <div className="flex items-center justify-center gap-4">
                  <Button
                    variant={isMuted ? "destructive" : "secondary"}
                    size="lg"
                    onClick={toggleMute}
                    className="w-12 h-12 rounded-full"
                  >
                    {isMuted ? <MicOff className="w-5 h-5" /> : <Mic className="w-5 h-5" />}
                  </Button>

                  <Button variant="secondary" size="lg" className="w-12 h-12 rounded-full">
                    <Settings className="w-5 h-5" />
                  </Button>

                  <Button
                    variant="destructive"
                    size="lg"
                    onClick={leaveChannel}
                    className="w-12 h-12 rounded-full"
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
            <div className="text-center py-8 text-muted-foreground">Odalar yükleniyor...</div>
          ) : channels.length === 0 ? (
            <Card>
              <CardContent className="pt-6 text-center text-muted-foreground">
                <p>Henüz sesli oda yok. Yeni bir oda oluşturun!</p>
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
                        <span className="text-muted-foreground">Katılımcı:</span>
                        <span>{channel.active_users} kişi</span>
                      </div>
                    </div>
                    <Button 
                      className="w-full mt-4" 
                      disabled={currentChannel?.id === channel.id}
                      onClick={() => joinChannel(channel)}
                    >
                      {currentChannel?.id === channel.id ? 'Bağlısın' : 'Katıl'}
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
