"use client";

import { useState } from 'react';
import { Search, Plus, MoreVertical, Send, Paperclip, Smile, Phone, Video, Users, ArrowLeft } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Input } from './ui/input';
import { Badge } from './ui/badge';
import { Textarea } from './ui/textarea';
import { ScrollArea } from './ui/scroll-area';

interface Chat {
  id: string;
  name: string;
  type: 'individual' | 'group';
  lastMessage: string;
  lastMessageTime: string;
  unreadCount: number;
  isOnline: boolean;
  participants?: string[];
  avatar: string;
}

interface Message {
  id: string;
  senderId: string;
  senderName: string;
  content: string;
  timestamp: string;
  type: 'text' | 'image' | 'file';
  isOwn: boolean;
}

const mockChats: Chat[] = [
  {
    id: '1',
    name: 'Dr. Ahmet Yılmaz',
    type: 'individual',
    lastMessage: 'Tez danışmanlığı toplantısı için yarın 14:00 uygun mu?',
    lastMessageTime: '10:30',
    unreadCount: 2,
    isOnline: true,
    avatar: 'AY'
  },
  {
    id: '2',
    name: 'Bilgisayar Mühendisliği Grubu',
    type: 'group',
    lastMessage: 'Zeynep: Final projesi için ekip toplantısı yapalım',
    lastMessageTime: '09:45',
    unreadCount: 5,
    isOnline: false,
    participants: ['Ali Uzun', 'Zeynep Kaya', 'Mehmet Demir', 'Ayşe Çelik'],
    avatar: 'BMG'
  },
  {
    id: '3',
    name: 'Prof. Dr. Elif Kaya',
    type: 'individual',
    lastMessage: 'Araştırma önerinizdeki metodoloji kısmını gözden geçirebilir misiniz?',
    lastMessageTime: '08:20',
    unreadCount: 0,
    isOnline: false,
    avatar: 'EK'
  },
  {
    id: '4',
    name: 'Proje Geliştirme Ekibi',
    type: 'group',
    lastMessage: 'Can: Demo sunumu için hazırlıklar nasıl gidiyor?',
    lastMessageTime: 'Dün',
    unreadCount: 3,
    isOnline: false,
    participants: ['Ali Uzun', 'Can Özkan', 'Deniz Yılmaz'],
    avatar: 'PGE'
  }
];

const mockMessages: Message[] = [
  {
    id: '1',
    senderId: 'dr_ahmet',
    senderName: 'Dr. Ahmet Yılmaz',
    content: 'Merhaba Ali, tez çalışman nasıl gidiyor?',
    timestamp: '09:30',
    type: 'text',
    isOwn: false
  },
  {
    id: '2',
    senderId: 'ali',
    senderName: 'Ali Uzun',
    content: 'Merhaba hocam, literatür taraması kısmını bitirdim. Metodoloji bölümüne geçmeye hazırlanıyorum.',
    timestamp: '09:35',
    type: 'text',
    isOwn: true
  },
  {
    id: '3',
    senderId: 'dr_ahmet',
    senderName: 'Dr. Ahmet Yılmaz',
    content: 'Harika! Metodoloji için yarın bir toplantı ayarlayalım. Saat 14:00 uygun mu?',
    timestamp: '10:30',
    type: 'text',
    isOwn: false
  }
];

export function Muhabbet() {
  const [selectedChat, setSelectedChat] = useState<Chat | null>(null);
  const [searchTerm, setSearchTerm] = useState('');
  const [newMessage, setNewMessage] = useState('');
  const [messages] = useState<Message[]>(mockMessages);

  const filteredChats = mockChats.filter(chat =>
    chat.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
    chat.lastMessage.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const sendMessage = () => {
    if (newMessage.trim()) {
      // Here you would typically send the message to your backend
      setNewMessage('');
    }
  };

  const handleKeyPress = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      sendMessage();
    }
  };

  return (
    <div className="h-full flex flex-col bg-background pb-20">
      {selectedChat ? (
        /* Chat View */
        <div className="flex-1 flex flex-col">
          {/* Chat Header */}
          <div className="p-4 border-b border-border bg-background/95 backdrop-blur-sm">
            <div className="flex items-center justify-between">
              <div className="flex items-center gap-3">
                <Button
                  variant="ghost"
                  size="sm"
                  onClick={() => setSelectedChat(null)}
                  className="p-2"
                >
                  <ArrowLeft className="w-4 h-4" />
                </Button>
                <div className="relative">
                  <div className="w-10 h-10 bg-secondary rounded-full flex items-center justify-center">
                    <span className="text-secondary-foreground text-sm">{selectedChat.avatar}</span>
                  </div>
                  {selectedChat.type === 'individual' && selectedChat.isOnline && (
                    <div className="w-3 h-3 bg-primary rounded-full absolute -bottom-0.5 -right-0.5 border-2 border-background"></div>
                  )}
                  {selectedChat.type === 'group' && (
                    <Users className="w-4 h-4 bg-primary text-primary-foreground rounded-full p-0.5 absolute -bottom-0.5 -right-0.5" />
                  )}
                </div>
                <div>
                  <h3 className="font-medium">{selectedChat.name}</h3>
                  {selectedChat.type === 'individual' ? (
                    <p className="text-sm text-muted-foreground">
                      {selectedChat.isOnline ? 'Çevrimiçi' : 'Son görülme: 2 saat önce'}
                    </p>
                  ) : (
                    <p className="text-sm text-muted-foreground">
                      {selectedChat.participants?.length} üye
                    </p>
                  )}
                </div>
              </div>
              
              <div className="flex items-center gap-2">
                <Button variant="ghost" size="sm">
                  <Phone className="w-4 h-4" />
                </Button>
                <Button variant="ghost" size="sm">
                  <Video className="w-4 h-4" />
                </Button>
                <Button variant="ghost" size="sm">
                  <MoreVertical className="w-4 h-4" />
                </Button>
              </div>
            </div>
          </div>

          {/* Messages */}
          <ScrollArea className="flex-1 p-4">
            <div className="space-y-4">
              {messages.map((message) => (
                <div
                  key={message.id}
                  className={`flex ${message.isOwn ? 'justify-end' : 'justify-start'}`}
                >
                  <div className={`max-w-[85%] ${message.isOwn ? 'order-last' : ''}`}>
                    {!message.isOwn && (
                      <p className="text-xs text-muted-foreground mb-1 ml-3">
                        {message.senderName}
                      </p>
                    )}
                    <div
                      className={`p-3 rounded-2xl ${
                        message.isOwn
                          ? 'bg-primary text-primary-foreground rounded-br-md'
                          : 'bg-muted rounded-bl-md'
                      }`}
                    >
                      <p className="whitespace-pre-wrap text-sm">{message.content}</p>
                      <p className={`text-xs mt-1 ${
                        message.isOwn 
                          ? 'text-primary-foreground/70' 
                          : 'text-muted-foreground'
                      }`}>
                        {message.timestamp}
                      </p>
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </ScrollArea>

          {/* Message Input */}
          <div className="p-4 border-t border-border bg-background">
            <div className="flex items-end gap-2">
              <Button variant="ghost" size="sm">
                <Paperclip className="w-4 h-4" />
              </Button>
              
              <div className="flex-1">
                <Textarea
                  placeholder="Mesaj yaz..."
                  value={newMessage}
                  onChange={(e) => setNewMessage(e.target.value)}
                  onKeyPress={handleKeyPress}
                  className="min-h-10 max-h-32 resize-none"
                  rows={1}
                />
              </div>
              
              <Button variant="ghost" size="sm">
                <Smile className="w-4 h-4" />
              </Button>
              
              <Button onClick={sendMessage} disabled={!newMessage.trim()}>
                <Send className="w-4 h-4" />
              </Button>
            </div>
          </div>
        </div>
      ) : (
        /* Chat List */
        <div className="flex-1 flex flex-col">
          {/* Header */}
          <div className="sticky top-0 bg-background/95 backdrop-blur-sm z-10 p-4 -mx-4 border-b border-border">
            <div className="px-4">
              <div className="flex items-center justify-between mb-4">
                <h2 className="text-primary">Muhabbet</h2>
                <Button size="sm">
                  <Plus className="w-4 h-4 mr-2" />
                  Yeni
                </Button>
              </div>
            
              {/* Search */}
              <div className="relative">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-muted-foreground w-4 h-4" />
                <Input
                  placeholder="Sohbet ara..."
                  value={searchTerm}
                  onChange={(e) => setSearchTerm(e.target.value)}
                  className="pl-10"
                />
              </div>
            </div>
          </div>

          {/* Chat List */}
          <ScrollArea className="flex-1">
            <div className="p-2">
              {filteredChats.map((chat) => (
                <div
                  key={chat.id}
                  onClick={() => setSelectedChat(chat)}
                  className="p-3 rounded-lg cursor-pointer transition-colors hover:bg-accent/50 active:bg-accent"
                >
                  <div className="flex items-start gap-3">
                    <div className="relative">
                      <div className="w-12 h-12 bg-secondary rounded-full flex items-center justify-center">
                        <span className="text-secondary-foreground">{chat.avatar}</span>
                      </div>
                      {chat.type === 'individual' && chat.isOnline && (
                        <div className="w-3 h-3 bg-primary rounded-full absolute -bottom-0.5 -right-0.5 border-2 border-background"></div>
                      )}
                      {chat.type === 'group' && (
                        <Users className="w-4 h-4 bg-primary text-primary-foreground rounded-full p-0.5 absolute -bottom-0.5 -right-0.5" />
                      )}
                    </div>
                    
                    <div className="flex-1 min-w-0">
                      <div className="flex items-center justify-between mb-1">
                        <h4 className="font-medium truncate">{chat.name}</h4>
                        <div className="flex items-center gap-2 flex-shrink-0">
                          <span className="text-xs text-muted-foreground">{chat.lastMessageTime}</span>
                          {chat.unreadCount > 0 && (
                            <Badge className="bg-primary text-primary-foreground min-w-[20px] h-5 flex items-center justify-center text-xs">
                              {chat.unreadCount}
                            </Badge>
                          )}
                        </div>
                      </div>
                      <p className="text-sm text-muted-foreground truncate">{chat.lastMessage}</p>
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </ScrollArea>
        </div>
      )}
    </div>
  );
}
