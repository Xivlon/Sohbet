"use client";

import { useState } from 'react';
import { Mic, MicOff, PhoneCall, PhoneOff, Users, Volume2, Settings } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from './ui/tabs';
import { Badge } from './ui/badge';
import { Slider } from './ui/slider';

interface VoiceRoom {
  id: string;
  name: string;
  description: string;
  participants: number;
  maxParticipants: number;
  isActive: boolean;
  type: 'local' | 'global';
  university?: string;
  topic: string;
  moderator: string;
}

interface Participant {
  id: string;
  name: string;
  university: string;
  isMuted: boolean;
  isSpeaking: boolean;
  isModerator: boolean;
}

const mockLocalRooms: VoiceRoom[] = [
  {
    id: '1',
    name: 'Bilgisayar Mühendisliği Sohbeti',
    description: 'Güncel teknoloji konularını tartışıyoruz',
    participants: 12,
    maxParticipants: 20,
    isActive: true,
    type: 'local',
    university: 'İTÜ',
    topic: 'Teknoloji',
    moderator: 'Dr. Ahmet Yılmaz'
  },
  {
    id: '2',
    name: 'Tez Yazım Süreçleri',
    description: 'Lisansüstü öğrenciler için tez yazım deneyimleri',
    participants: 8,
    maxParticipants: 15,
    isActive: true,
    type: 'local',
    university: 'İTÜ',
    topic: 'Akademik',
    moderator: 'Prof. Dr. Elif Kaya'
  },
  {
    id: '3',
    name: 'Proje Geliştirme Atölyesi',
    description: 'Öğrenci projeleri üzerine fikir alışverişi',
    participants: 5,
    maxParticipants: 25,
    isActive: false,
    type: 'local',
    university: 'İTÜ',
    topic: 'Proje',
    moderator: 'Asst. Prof. Can Özkan'
  }
];

const mockGlobalRooms: VoiceRoom[] = [
  {
    id: '4',
    name: 'International AI Discussions',
    description: 'Global AI researchers sharing insights',
    participants: 45,
    maxParticipants: 100,
    isActive: true,
    type: 'global',
    topic: 'Artificial Intelligence',
    moderator: 'Prof. Sarah Johnson (MIT)'
  },
  {
    id: '5',
    name: 'Climate Research Collaboration',
    description: 'Universities worldwide discussing climate solutions',
    participants: 32,
    maxParticipants: 50,
    isActive: true,
    type: 'global',
    topic: 'Climate Science',
    moderator: 'Dr. Lars Hansen (ETH Zurich)'
  },
  {
    id: '6',
    name: 'Student Exchange Experiences',
    description: 'Students sharing international study experiences',
    participants: 18,
    maxParticipants: 30,
    isActive: true,
    type: 'global',
    topic: 'Student Life',
    moderator: 'Maria Garcia (University of Barcelona)'
  }
];

const mockParticipants: Participant[] = [
  { id: '1', name: 'Ali Uzun', university: 'İTÜ', isMuted: false, isSpeaking: true, isModerator: false },
  { id: '2', name: 'Dr. Ahmet Yılmaz', university: 'İTÜ', isMuted: false, isSpeaking: false, isModerator: true },
  { id: '3', name: 'Zeynep Kaya', university: 'İTÜ', isMuted: true, isSpeaking: false, isModerator: false },
  { id: '4', name: 'Mehmet Demir', university: 'İTÜ', isMuted: false, isSpeaking: false, isModerator: false }
];

export function Khave() {
  const [activeTab, setActiveTab] = useState<'local' | 'global'>('local');
  const [currentRoom, setCurrentRoom] = useState<VoiceRoom | null>(null);
  const [isMuted, setIsMuted] = useState(false);
  const [isConnected, setIsConnected] = useState(false);
  const [volume, setVolume] = useState([75]);

  const rooms = activeTab === 'local' ? mockLocalRooms : mockGlobalRooms;

  const joinRoom = (room: VoiceRoom) => {
    setCurrentRoom(room);
    setIsConnected(true);
  };

  const leaveRoom = () => {
    setCurrentRoom(null);
    setIsConnected(false);
    setIsMuted(false);
  };

  const toggleMute = () => {
    setIsMuted(!isMuted);
  };

  return (
    <div className="h-full overflow-y-auto bg-background">
      <div className="max-w-full mx-auto p-4 pb-20">
        {/* Header */}
        <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 mb-4 border-b border-border">
          <h2 className="text-primary">Khave - Sesli Sohbet</h2>
          <p className="text-muted-foreground text-sm">Gerçek zamanlı sesli tartışma odaları</p>
        </div>

        {/* Current Room (if connected) */}
        {currentRoom && isConnected && (
          <Card className="mb-6 border-primary">
            <CardHeader>
              <div className="flex items-center justify-between">
                <div>
                  <h3 className="flex items-center gap-2">
                    <div className="w-3 h-3 bg-green-500 rounded-full animate-pulse"></div>
                    {currentRoom.name}
                  </h3>
                  <p className="text-muted-foreground text-sm">{currentRoom.description}</p>
                </div>
                <Badge variant="secondary">
                  {currentRoom.participants} / {currentRoom.maxParticipants} kişi
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
                    onClick={leaveRoom}
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

        {/* Room Tabs */}
        <Tabs value={activeTab} onValueChange={(value) => setActiveTab(value as 'local' | 'global')}>
          <TabsList className="grid w-full grid-cols-2 mb-6">
            <TabsTrigger value="local">Yerel Üniversite</TabsTrigger>
            <TabsTrigger value="global">Global Etkileşim</TabsTrigger>
          </TabsList>

          <TabsContent value="local" className="space-y-4">
            <div className="grid grid-cols-1 gap-4">
              {mockLocalRooms.map((room) => (
                <Card key={room.id} className={`cursor-pointer transition-colors ${
                  room.isActive ? 'hover:bg-accent' : 'opacity-60'
                }`}>
                  <CardHeader className="pb-3">
                    <div className="flex items-center justify-between">
                      <div className="flex items-center gap-2">
                        <div className={`w-3 h-3 rounded-full ${
                          room.isActive ? 'bg-green-500' : 'bg-gray-400'
                        }`}></div>
                        <h3 className="font-medium text-sm">{room.name}</h3>
                      </div>
                      <Badge variant="outline">{room.topic}</Badge>
                    </div>
                  </CardHeader>
                  <CardContent className="pt-0">
                    <p className="text-muted-foreground text-sm mb-3">{room.description}</p>
                    <div className="space-y-2">
                      <div className="flex items-center justify-between text-sm">
                        <span className="text-muted-foreground">Moderatör:</span>
                        <span>{room.moderator}</span>
                      </div>
                      <div className="flex items-center justify-between text-sm">
                        <span className="text-muted-foreground">Katılımcı:</span>
                        <span>{room.participants} / {room.maxParticipants}</span>
                      </div>
                    </div>
                    <Button 
                      className="w-full mt-4" 
                      disabled={!room.isActive || currentRoom?.id === room.id}
                      onClick={() => room.isActive && joinRoom(room)}
                    >
                      {currentRoom?.id === room.id ? 'Bağlısın' : room.isActive ? 'Katıl' : 'Aktif Değil'}
                    </Button>
                  </CardContent>
                </Card>
              ))}
            </div>
          </TabsContent>

          <TabsContent value="global" className="space-y-4">
            <div className="grid grid-cols-1 gap-4">
              {mockGlobalRooms.map((room) => (
                <Card key={room.id} className="cursor-pointer hover:bg-accent transition-colors">
                  <CardHeader className="pb-3">
                    <div className="flex items-center justify-between">
                      <div className="flex items-center gap-2">
                        <div className="w-3 h-3 bg-green-500 rounded-full"></div>
                        <h3 className="font-medium text-sm">{room.name}</h3>
                      </div>
                      <Badge variant="outline">{room.topic}</Badge>
                    </div>
                  </CardHeader>
                  <CardContent className="pt-0">
                    <p className="text-muted-foreground text-sm mb-3">{room.description}</p>
                    <div className="space-y-2">
                      <div className="flex items-center justify-between text-sm">
                        <span className="text-muted-foreground">Moderatör:</span>
                        <span className="text-right">{room.moderator}</span>
                      </div>
                      <div className="flex items-center justify-between text-sm">
                        <span className="text-muted-foreground">Katılımcı:</span>
                        <span>{room.participants} / {room.maxParticipants}</span>
                      </div>
                    </div>
                    <Button 
                      className="w-full mt-4"
                      disabled={currentRoom?.id === room.id}
                      onClick={() => joinRoom(room)}
                    >
                      {currentRoom?.id === room.id ? 'Bağlısın' : 'Katıl'}
                    </Button>
                  </CardContent>
                </Card>
              ))}
            </div>
          </TabsContent>
        </Tabs>
      </div>
    </div>
  );
}
