"use client";

import { useState, useEffect } from 'react';
import { Search, Plus, MoreVertical, Send, Paperclip, Smile, Phone, Video, Users, ArrowLeft } from 'lucide-react';
import { Card, CardContent, CardHeader } from './ui/card';
import { Button } from './ui/button';
import { Input } from './ui/input';
import { Badge } from './ui/badge';
import { Textarea } from './ui/textarea';
import { ScrollArea } from './ui/scroll-area';

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
  const [currentUserId] = useState<number>(1); // TODO: Get from auth context
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchConversations();
  }, []);

  useEffect(() => {
    if (selectedChat) {
      fetchMessages(selectedChat.id);
    }
  }, [selectedChat]);

  const fetchConversations = async () => {
    try {
      const response = await fetch('/api/conversations', {
        headers: {
          'X-User-ID': currentUserId.toString()
        }
      });
      
      if (response.ok) {
        const data = await response.json();
        
        // Fetch user details for each conversation
        const conversationsWithUsers = await Promise.all(
          data.conversations.map(async (conv: Chat) => {
            const otherUserId = conv.user1_id === currentUserId ? conv.user2_id : conv.user1_id;
            
            try {
              const userResponse = await fetch(`/api/users/${otherUserId}`);
              if (userResponse.ok) {
                const userData = await userResponse.json();
                return {
                  ...conv,
                  other_user: {
                    id: otherUserId,
                    username: userData.username || 'Unknown User'
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
                username: 'Unknown User'
              }
            };
          })
        );
        
        setChats(conversationsWithUsers);
      }
    } catch (error) {
      console.error('Error fetching conversations:', error);
    } finally {
      setLoading(false);
    }
  };

  const fetchMessages = async (conversationId: number) => {
    try {
      const response = await fetch(`/api/conversations/${conversationId}/messages`, {
        headers: {
          'X-User-ID': currentUserId.toString()
        }
      });
      
      if (response.ok) {
        const data = await response.json();
        // Reverse to show oldest first
        setMessages(data.messages.reverse());
      }
    } catch (error) {
      console.error('Error fetching messages:', error);
    }
  };

  const filteredChats = chats.filter(chat =>
    chat.other_user?.username.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const sendMessage = async () => {
    if (newMessage.trim() && selectedChat) {
      try {
        const response = await fetch(`/api/conversations/${selectedChat.id}/messages`, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            'X-User-ID': currentUserId.toString()
          },
          body: JSON.stringify({
            content: newMessage.trim()
          })
        });
        
        if (response.ok) {
          const message = await response.json();
          setMessages([...messages, message]);
          setNewMessage('');
          
          // Update conversation's last_message_at in the list
          setChats(chats.map(chat => 
            chat.id === selectedChat.id 
              ? { ...chat, last_message_at: new Date().toISOString() }
              : chat
          ));
        }
      } catch (error) {
        console.error('Error sending message:', error);
      }
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
                  <h3 className="font-medium">{selectedChat.other_user?.username || 'Unknown User'}</h3>
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
              {loading ? (
                <div className="p-4 text-center text-muted-foreground">
                  Yükleniyor...
                </div>
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
                          <h4 className="font-medium truncate">{chat.other_user?.username || 'Unknown User'}</h4>
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
