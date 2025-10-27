"use client";

import { useState, useEffect } from 'react';
import { Search, Plus, MoreVertical, Send, Paperclip, Smile, Phone, Video, ArrowLeft } from 'lucide-react';
import { Card, CardContent } from './ui/card';
import { Button } from './ui/button';
import { Input } from './ui/input';
import { Textarea } from './ui/textarea';
import { ScrollArea } from './ui/scroll-area';
import { apiClient } from '@/app/lib/api-client';
import { useAuth } from '@/app/contexts/auth-context';

interface Chat {
  id: number;
  user1_id: number;
  user2_id: number;
  created_at: string;
  last_message_at: string;
  other_user?: {
    id: number;
    username: string;
  };
}

interface Message {
  id: number;
  conversation_id: number;
  sender_id: number;
  content: string;
  media_url?: string;
  read_at?: string;
  delivered_at?: string;
  created_at: string;
}

export function Muhabbet() {
  const [selectedChat, setSelectedChat] = useState<Chat | null>(null);
  const [searchTerm, setSearchTerm] = useState('');
  const [newMessage, setNewMessage] = useState('');
  const [chats, setChats] = useState<Chat[]>([]);
  const [messages, setMessages] = useState<Message[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [sendingMessage, setSendingMessage] = useState(false);
  
  const { user } = useAuth();
  const currentUserId = user?.id;

  useEffect(() => {
    if (currentUserId) {
      fetchConversations();
    }
  }, [currentUserId]);

  useEffect(() => {
    if (selectedChat && currentUserId) {
      fetchMessages(selectedChat.id);
    }
  }, [selectedChat, currentUserId]);

  const fetchConversations = async () => {
    if (!currentUserId) return;
    
    setLoading(true);
    setError(null);
    
    try {
      const response = await apiClient.getConversations();
      
      if (response.data) {
        const conversations = response.data.conversations || [];
        
        // Fetch user details for each conversation
        const conversationsWithUsers = await Promise.all(
          conversations.map(async (conv: Chat) => {
            const otherUserId = conv.user1_id === currentUserId ? conv.user2_id : conv.user1_id;
            
            try {
              const userResponse = await apiClient.getUserById(otherUserId);
              if (userResponse && userResponse.data) {
                return {
                  ...conv,
                  other_user: {
                    id: otherUserId,
                    username: userResponse.data.username || 'Bilinmeyen Kullanıcı'
                  }
                };
              }
            } catch (error) {
              console.error('Error fetching user:', error);
            }
            
            return {
              ...conv,
              other_user: {
                id: otherUserId,
                username: 'Bilinmeyen Kullanıcı'
              }
            };
          })
        );
        
        setChats(conversationsWithUsers);
      } else {
        setError(response.error || 'Sohbetler yüklenemedi');
      }
    } catch (err) {
      setError('Bir hata oluştu. Lütfen tekrar deneyin.');
      console.error('Error fetching conversations:', err);
    } finally {
      setLoading(false);
    }
  };

  const fetchMessages = async (conversationId: number) => {
    if (!currentUserId) return;
    
    try {
      const response = await apiClient.getMessages(conversationId, 100, 0);
      
      if (response.data) {
        // Reverse to show oldest first
        setMessages(response.data.messages.reverse());
      }
    } catch (error) {
      console.error('Error fetching messages:', error);
    }
  };

  const filteredChats = chats.filter(chat =>
    chat.other_user?.username.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const sendMessage = async () => {
    if (!newMessage.trim() || !selectedChat || !currentUserId) return;

    setSendingMessage(true);
    const messageContent = newMessage.trim();
    
    // Optimistic update
    const tempMessage: Message = {
      id: Date.now(),
      conversation_id: selectedChat.id,
      sender_id: currentUserId,
      content: messageContent,
      created_at: new Date().toISOString()
    };
    
    setMessages([...messages, tempMessage]);
    setNewMessage('');
    
    try {
      const response = await apiClient.sendMessage(selectedChat.id, messageContent);
      
      if (response.data) {
        // Replace temp message with real one
        setMessages(prev => 
          prev.map(msg => msg.id === tempMessage.id ? response.data : msg)
        );
        
        // Update conversation's last_message_at
        setChats(chats.map(chat => 
          chat.id === selectedChat.id 
            ? { ...chat, last_message_at: new Date().toISOString() }
            : chat
        ));
      } else {
        // Remove temp message on error
        setMessages(prev => prev.filter(msg => msg.id !== tempMessage.id));
        alert('Mesaj gönderilemedi: ' + (response.error || 'Bilinmeyen hata'));
      }
    } catch (err) {
      // Remove temp message on error
      setMessages(prev => prev.filter(msg => msg.id !== tempMessage.id));
      alert('Mesaj gönderilemedi. Lütfen tekrar deneyin.');
      console.error('Error sending message:', err);
    } finally {
      setSendingMessage(false);
    }
  };

  const handleKeyPress = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      sendMessage();
    }
  };

  const formatTimestamp = (timestamp: string) => {
    const date = new Date(timestamp);
    const now = new Date();
    const diffInHours = (now.getTime() - date.getTime()) / (1000 * 60 * 60);
    
    if (diffInHours < 24) {
      return date.toLocaleTimeString('tr-TR', { hour: '2-digit', minute: '2-digit' });
    } else if (diffInHours < 48) {
      return 'Dün';
    } else {
      return date.toLocaleDateString('tr-TR', { month: 'short', day: 'numeric' });
    }
  };

  if (!currentUserId) {
    return (
      <div className="h-full flex items-center justify-center">
        <p className="text-muted-foreground">Lütfen giriş yapın</p>
      </div>
    );
  }

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
                    <span className="text-secondary-foreground text-sm">
                      {selectedChat.other_user?.username?.charAt(0).toUpperCase() || 'U'}
                    </span>
                  </div>
                </div>
                <div>
                  <h3 className="font-medium">{selectedChat.other_user?.username || 'Bilinmeyen Kullanıcı'}</h3>
                  <p className="text-sm text-muted-foreground">
                    Son görülme: {formatTimestamp(selectedChat.last_message_at)}
                  </p>
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
              {messages.map((message) => {
                const isOwn = message.sender_id === currentUserId;
                
                return (
                  <div
                    key={message.id}
                    className={`flex ${isOwn ? 'justify-end' : 'justify-start'}`}
                  >
                    <div className={`max-w-[85%] ${isOwn ? 'order-last' : ''}`}>
                      <div
                        className={`p-3 rounded-2xl ${
                          isOwn
                            ? 'bg-primary text-primary-foreground rounded-br-md'
                            : 'bg-muted rounded-bl-md'
                        }`}
                      >
                        <p className="whitespace-pre-wrap text-sm">{message.content}</p>
                        <p className={`text-xs mt-1 ${
                          isOwn 
                            ? 'text-primary-foreground/70' 
                            : 'text-muted-foreground'
                        }`}>
                          {formatTimestamp(message.created_at)}
                        </p>
                      </div>
                    </div>
                  </div>
                );
              })}
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
                  disabled={sendingMessage}
                />
              </div>
              
              <Button variant="ghost" size="sm">
                <Smile className="w-4 h-4" />
              </Button>
              
              <Button 
                onClick={sendMessage} 
                disabled={!newMessage.trim() || sendingMessage}
              >
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
              {loading ? (
                <div className="p-4 text-center text-muted-foreground">
                  Yükleniyor...
                </div>
              ) : error ? (
                <Card>
                  <CardContent className="p-8 text-center">
                    <p className="text-destructive mb-4">{error}</p>
                    <Button onClick={fetchConversations}>Tekrar Dene</Button>
                  </CardContent>
                </Card>
              ) : filteredChats.length === 0 ? (
                <div className="p-4 text-center text-muted-foreground">
                  {searchTerm ? 'Sohbet bulunamadı' : 'Henüz mesajınız yok'}
                </div>
              ) : (
                filteredChats.map((chat) => (
                  <div
                    key={chat.id}
                    onClick={() => setSelectedChat(chat)}
                    className="p-3 rounded-lg cursor-pointer transition-colors hover:bg-accent/50 active:bg-accent"
                  >
                    <div className="flex items-start gap-3">
                      <div className="relative">
                        <div className="w-12 h-12 bg-secondary rounded-full flex items-center justify-center">
                          <span className="text-secondary-foreground">
                            {chat.other_user?.username?.charAt(0).toUpperCase() || 'U'}
                          </span>
                        </div>
                      </div>
                      
                      <div className="flex-1 min-w-0">
                        <div className="flex items-center justify-between mb-1">
                          <h4 className="font-medium truncate">{chat.other_user?.username || 'Bilinmeyen Kullanıcı'}</h4>
                          <span className="text-xs text-muted-foreground flex-shrink-0 ml-2">
                            {formatTimestamp(chat.last_message_at)}
                          </span>
                        </div>
                        <p className="text-sm text-muted-foreground truncate">
                          {new Date(chat.last_message_at).toLocaleDateString('tr-TR')}
                        </p>
                      </div>
                    </div>
                  </div>
                ))
              )}
            </div>
          </ScrollArea>
        </div>
      )}
    </div>
  );
}
