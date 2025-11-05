"use client";

import { useState, useEffect } from 'react';
import { Search, Plus, MoreVertical, Send, Paperclip, Smile, Phone, Video, ArrowLeft } from 'lucide-react';
import { Card, CardContent } from './ui/card';
import { Button } from './ui/button';
import { Input } from './ui/input';
import { Textarea } from './ui/textarea';
import { ScrollArea } from './ui/scroll-area';
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogDescription } from './ui/dialog';
import { apiClient, Conversation, Message as ApiMessage, User } from '@/app/lib/api-client';
import { useAuth } from '@/app/contexts/auth-context';
import { useTranslations } from 'next-intl';

interface Message extends ApiMessage {
  media_url?: string;
  read_at?: string;
  delivered_at?: string;
}

// Mock users for chat testing
const MOCK_USERS: User[] = [
  {
    id: 1001,
    username: 'ahmet_yilmaz',
    email: 'ahmet@example.com',
    name: 'Ahmet Yılmaz',
    position: 'Öğrenci',
    university: 'İstanbul Teknik Üniversitesi',
    department: 'Bilgisayar Mühendisliği',
    enrollment_year: 2021,
    primary_language: 'Türkçe',
  },
  {
    id: 1002,
    username: 'ayse_demir',
    email: 'ayse@example.com',
    name: 'Ayşe Demir',
    position: 'Öğrenci',
    university: 'Boğaziçi Üniversitesi',
    department: 'Elektrik-Elektronik Mühendisliği',
    enrollment_year: 2020,
    primary_language: 'Türkçe',
  },
  {
    id: 1003,
    username: 'mehmet_kaya',
    email: 'mehmet@example.com',
    name: 'Mehmet Kaya',
    position: 'Asistan',
    university: 'Orta Doğu Teknik Üniversitesi',
    department: 'Yazılım Mühendisliği',
    enrollment_year: 2019,
    primary_language: 'Türkçe',
  },
  {
    id: 1004,
    username: 'fatma_ozturk',
    email: 'fatma@example.com',
    name: 'Fatma Öztürk',
    position: 'Öğrenci',
    university: 'Hacettepe Üniversitesi',
    department: 'Bilgisayar Mühendisliği',
    enrollment_year: 2022,
    primary_language: 'Türkçe',
  },
  {
    id: 1005,
    username: 'can_arslan',
    email: 'can@example.com',
    name: 'Can Arslan',
    position: 'Öğrenci',
    university: 'İstanbul Üniversitesi',
    department: 'Bilişim Sistemleri',
    enrollment_year: 2021,
    primary_language: 'Türkçe',
  },
];

export function Muhabbet() {
  const [selectedChat, setSelectedChat] = useState<Conversation | null>(null);
  const [searchTerm, setSearchTerm] = useState('');
  const [newMessage, setNewMessage] = useState('');
  const [chats, setChats] = useState<Conversation[]>([]);
  const [messages, setMessages] = useState<Message[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [sendingMessage, setSendingMessage] = useState(false);
  const [showNewChatDialog, setShowNewChatDialog] = useState(false);

  const { user } = useAuth();
  const currentUserId = user?.id;

  const t = useTranslations('muhabbet');
  const tCommon = useTranslations('common');
  const tErrors = useTranslations('errors');

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
          conversations.map(async (conv: Conversation) => {
            const otherUserId = conv.user1_id === currentUserId ? conv.user2_id : conv.user1_id;
            
            try {
              const userResponse = await apiClient.getUserById(otherUserId);
              if (userResponse && userResponse.data) {
                return {
                  ...conv,
                  other_user: userResponse.data
                };
              }
            } catch (error) {
              console.error('Error fetching user:', error);
            }
            
            return {
              ...conv,
              other_user: {
                id: otherUserId,
                username: t('unknownUser'),
                email: ''
              }
            };
          })
        );
        
        setChats(conversationsWithUsers);
      } else {
        setError(response.error || t('chatsLoadError'));
      }
    } catch (err) {
      setError(tErrors('somethingWentWrong'));
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

    // Check if this is a mock user conversation
    const isMockUser = MOCK_USERS.some(u => u.id === selectedChat.other_user?.id);

    if (isMockUser) {
      // For mock users, just keep the message (no API call)
      setSendingMessage(false);

      // Update conversation's last_message_at
      setChats(chats.map(chat =>
        chat.id === selectedChat.id
          ? { ...chat, last_message_at: new Date().toISOString() }
          : chat
      ));

      // Simulate a response from the mock user after a delay
      simulateMockUserResponse(selectedChat, messageContent);
      return;
    }

    try {
      const response = await apiClient.sendMessage(selectedChat.id, messageContent);

      if (response.data) {
        // Replace temp message with real one
        setMessages(prev =>
          prev.map(msg => msg.id === tempMessage.id && response.data ? response.data : msg)
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
        alert(tErrors('somethingWentWrong') + ': ' + (response.error || tErrors('somethingWentWrong')));
      }
    } catch (err) {
      // Remove temp message on error
      setMessages(prev => prev.filter(msg => msg.id !== tempMessage.id));
      alert(tErrors('somethingWentWrong'));
      console.error('Error sending message:', err);
    } finally {
      setSendingMessage(false);
    }
  };

  const simulateMockUserResponse = (conversation: Conversation, userMessage: string) => {
    // Wait 1-3 seconds before responding
    const delay = 1000 + Math.random() * 2000;

    setTimeout(() => {
      const mockResponses = [
        'Çok ilginç! Devam et lütfen.',
        'Anladım, peki sen ne düşünüyorsun?',
        'Haklısın, ben de öyle düşünüyorum.',
        'Hmm, ilginç bir bakış açısı.',
        'Kesinlikle katılıyorum!',
        'Bunu hiç düşünmemiştim, güzel fikir.',
        'Evet, bence de öyle.',
        'Çok doğru söylüyorsun.',
        'Bu konuda sana tamamen katılıyorum.',
        'Gerçekten mi? Çok ilginç!',
        'Anlıyorum, mantıklı geliyor.',
        'Evet, bu konu gerçekten önemli.',
        'Harika bir gözlem!',
        'Bunu daha detaylı anlatır mısın?',
        'Tam olarak ne demek istediğini anlıyorum.',
      ];

      // If user asks a question, give a more specific response
      let response;
      if (userMessage.includes('?')) {
        const questionResponses = [
          'Bence bu konuda daha fazla araştırma yapmak gerekir.',
          'İyi soru! Düşünmem lazım biraz.',
          'Şu an kesin bir cevap veremem ama araştırırım.',
          'Bu çok iyi bir soru, birlikte düşünelim.',
          'Hmm, zor bir soru. Ne dersin sen?',
        ];
        response = questionResponses[Math.floor(Math.random() * questionResponses.length)];
      } else {
        response = mockResponses[Math.floor(Math.random() * mockResponses.length)];
      }

      const mockMessage: Message = {
        id: Date.now(),
        conversation_id: conversation.id,
        sender_id: conversation.other_user!.id,
        content: response,
        created_at: new Date().toISOString(),
      };

      setMessages(prev => [...prev, mockMessage]);

      // Update conversation's last_message_at
      setChats(prev => prev.map(chat =>
        chat.id === conversation.id
          ? { ...chat, last_message_at: new Date().toISOString() }
          : chat
      ));
    }, delay);
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

  const createMockConversation = async (mockUser: User) => {
    if (!currentUserId) return;

    // Check if conversation already exists with this mock user
    const existingChat = chats.find(
      chat => chat.other_user?.id === mockUser.id
    );

    if (existingChat) {
      // If conversation exists, just select it
      setSelectedChat(existingChat);
      setShowNewChatDialog(false);
      return;
    }

    // Create a new mock conversation
    const newConversation: Conversation = {
      id: Date.now(), // Temporary ID for mock conversation
      user1_id: currentUserId,
      user2_id: mockUser.id,
      created_at: new Date().toISOString(),
      last_message_at: new Date().toISOString(),
      other_user: mockUser,
    };

    // Add to chats list
    setChats([newConversation, ...chats]);
    setSelectedChat(newConversation);
    setShowNewChatDialog(false);

    // Send a welcome message from the mock user after a short delay
    setTimeout(() => {
      const welcomeMessages = [
        'Merhaba! Nasılsın?',
        'Selam, ne var ne yok?',
        'Hey! Seni tanımak güzel :)',
        'Merhaba, nasıl gidiyor?',
        'Selam! Yardımcı olabileceğim bir şey var mı?',
      ];

      const randomMessage = welcomeMessages[Math.floor(Math.random() * welcomeMessages.length)];

      const mockMessage: Message = {
        id: Date.now(),
        conversation_id: newConversation.id,
        sender_id: mockUser.id,
        content: randomMessage,
        created_at: new Date().toISOString(),
      };

      setMessages([mockMessage]);
    }, 1000);
  };

  if (!currentUserId) {
    return (
      <div className="h-full flex items-center justify-center">
        <p className="text-muted-foreground">{tCommon('loading')}</p>
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
                  <h3 className="font-medium">{selectedChat.other_user?.username || t('unknownUser')}</h3>
                  <p className="text-sm text-muted-foreground">
                    {selectedChat.last_message_at ? formatTimestamp(selectedChat.last_message_at) : t('offline')}
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
                  placeholder={t('typeMessage')}
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
                <h2 className="text-primary">{t('chats')}</h2>
                <Button size="sm" onClick={() => setShowNewChatDialog(true)}>
                  <Plus className="w-4 h-4 mr-2" />
                  {t('newChat')}
                </Button>
              </div>

              {/* Search */}
              <div className="relative">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-muted-foreground w-4 h-4" />
                <Input
                  placeholder={t('searchChats')}
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
                  {tCommon('loading')}
                </div>
              ) : error ? (
                <Card>
                  <CardContent className="p-8 text-center">
                    <p className="text-destructive mb-4">{error}</p>
                    <Button onClick={fetchConversations}>{tErrors('tryAgain')}</Button>
                  </CardContent>
                </Card>
              ) : filteredChats.length === 0 ? (
                <div className="p-4 text-center text-muted-foreground">
                  {searchTerm ? tErrors('notFound') : t('messages')}
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
                          <h4 className="font-medium truncate">{chat.other_user?.username || t('unknownUser')}</h4>
                          <span className="text-xs text-muted-foreground flex-shrink-0 ml-2">
                            {chat.last_message_at ? formatTimestamp(chat.last_message_at) : ''}
                          </span>
                        </div>
                        <p className="text-sm text-muted-foreground truncate">
                          {chat.last_message_at ? new Date(chat.last_message_at).toLocaleDateString('tr-TR') : t('messages')}
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

      {/* New Chat Dialog */}
      <Dialog open={showNewChatDialog} onOpenChange={setShowNewChatDialog}>
        <DialogContent className="max-w-md">
          <DialogHeader>
            <DialogTitle>{t('newChat')}</DialogTitle>
            <DialogDescription>
              {t('searchChats')}
            </DialogDescription>
          </DialogHeader>

          <ScrollArea className="max-h-96 mt-4">
            <div className="space-y-2">
              {MOCK_USERS.map((mockUser) => (
                <div
                  key={mockUser.id}
                  onClick={() => createMockConversation(mockUser)}
                  className="p-3 rounded-lg cursor-pointer transition-colors hover:bg-accent/50 active:bg-accent border border-border"
                >
                  <div className="flex items-center gap-3">
                    <div className="w-12 h-12 bg-primary/10 rounded-full flex items-center justify-center">
                      <span className="text-primary font-semibold">
                        {mockUser.username.charAt(0).toUpperCase()}
                      </span>
                    </div>

                    <div className="flex-1 min-w-0">
                      <h4 className="font-medium truncate">{mockUser.name || mockUser.username}</h4>
                      <p className="text-sm text-muted-foreground truncate">
                        @{mockUser.username}
                      </p>
                      {mockUser.university && (
                        <p className="text-xs text-muted-foreground truncate">
                          {mockUser.university} - {mockUser.department}
                        </p>
                      )}
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </ScrollArea>
        </DialogContent>
      </Dialog>
    </div>
  );
}
