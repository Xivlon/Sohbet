"use client"

import { useEffect, useState, useRef } from 'react'
import { CardContent, CardHeader, CardTitle } from '@/app/components/ui/card'
import { Button } from '@/app/components/ui/button'
import { Input } from '@/app/components/ui/input'
import { ScrollArea } from '@/app/components/ui/scroll-area'
import { Avatar, AvatarFallback } from '@/app/components/avatar'
import { Send } from 'lucide-react'
import { useChatWebSocket, ChatMessagePayload } from '../lib/use-websocket'
import { apiClient } from '@/app/lib/api-client'

interface Message extends ChatMessagePayload {
  id: number
  media_url?: string
  read_at?: string
  delivered_at?: string
  created_at: string
}

interface ChatWindowProps {
  conversationId: number
  currentUserId: number
  otherUser: {
    id: number
    username: string
  }
}

export function ChatWindow({ conversationId, currentUserId, otherUser }: ChatWindowProps) {
  const [messages, setMessages] = useState<Message[]>([])
  const [newMessage, setNewMessage] = useState('')
  const [loading, setLoading] = useState(true)
  const [sending, setSending] = useState(false)
  const scrollAreaRef = useRef<HTMLDivElement>(null)

  // Callback to handle incoming WebSocket messages
  const handleIncomingMessage = (incomingMessage: ChatMessagePayload) => {
    console.log('Adding incoming message to UI:', incomingMessage)
    // Only add messages with id
    if (!incomingMessage.id) {
      console.warn('Received message without id, skipping')
      return
    }
    const message: Message = {
      ...incomingMessage,
      id: incomingMessage.id,
      created_at: incomingMessage.created_at || new Date().toISOString()
    }
    setMessages(prev => {
      // Avoid duplicates by checking if message already exists
      const exists = prev.some(m => m.id === message.id)
      if (exists) {
        return prev
      }
      return [...prev, message]
    })
  }

  // Use WebSocket hook for real-time features
  const { sendMessage: sendWebSocketMessage, sendTyping, typingUsers } = useChatWebSocket(
    conversationId,
    handleIncomingMessage
  )
  
  // Check if other user is typing
  const isOtherUserTyping = typingUsers.includes(otherUser.id)

  useEffect(() => {
    fetchMessages()
  }, [conversationId])

  useEffect(() => {
    // Auto-scroll to bottom when messages change
    scrollToBottom()
  }, [messages])

  const fetchMessages = async () => {
    try {
      const response = await apiClient.getMessages(conversationId, 50, 0)
      
      if (response.data) {
        // Reverse to show oldest first
        setMessages(response.data.messages.reverse())
      }
    } catch (error) {
      console.error('Error fetching messages:', error)
    } finally {
      setLoading(false)
    }
  }

  const scrollToBottom = () => {
    if (scrollAreaRef.current) {
      const scrollElement = scrollAreaRef.current.querySelector('[data-radix-scroll-area-viewport]')
      if (scrollElement) {
        scrollElement.scrollTop = scrollElement.scrollHeight
      }
    }
  }

  const handleSendMessage = async (e: React.FormEvent) => {
    e.preventDefault()
    
    if (!newMessage.trim() || sending) return
    
    const messageContent = newMessage.trim()
    setSending(true)
    
    try {
      // Try to send via WebSocket first
      const webSocketSent = sendWebSocketMessage(messageContent)
      
      // Fallback to REST API if WebSocket fails
      if (!webSocketSent) {
        const response = await apiClient.sendMessage(conversationId, messageContent)
        
        if (response.data) {
          setMessages([...messages, response.data])
        }
      } else {
        // Optimistically add message to UI (WebSocket will confirm)
        const tempMessage: Message = {
          id: Date.now(), // Temporary ID
          conversation_id: conversationId,
          sender_id: currentUserId,
          content: messageContent,
          created_at: new Date().toISOString()
        }
        setMessages([...messages, tempMessage])
      }
      
      setNewMessage('')
    } catch (error) {
      console.error('Error sending message:', error)
    } finally {
      setSending(false)
    }
  }

  const handleTyping = () => {
    // Send typing indicator via WebSocket
    sendTyping()
  }

  const formatTimestamp = (timestamp: string) => {
    const date = new Date(timestamp)
    const now = new Date()
    const diffInHours = (now.getTime() - date.getTime()) / (1000 * 60 * 60)
    
    if (diffInHours < 24) {
      return date.toLocaleTimeString('tr-TR', { hour: '2-digit', minute: '2-digit' })
    } else {
      return date.toLocaleDateString('tr-TR', { month: 'short', day: 'numeric' })
    }
  }

  if (loading) {
    return (
      <div className="h-full flex items-center justify-center text-muted-foreground">
        Yükleniyor...
      </div>
    )
  }

  return (
    <div className="h-full flex flex-col">
      <CardHeader className="border-b">
        <CardTitle className="flex items-center gap-3">
          <Avatar>
            <AvatarFallback>
              {otherUser.username.charAt(0).toUpperCase()}
            </AvatarFallback>
          </Avatar>
          <div className="flex flex-col">
            <span>{otherUser.username}</span>
            {isOtherUserTyping && (
              <span className="text-xs text-muted-foreground font-normal">
                yazıyor...
              </span>
            )}
          </div>
        </CardTitle>
      </CardHeader>
      
      <ScrollArea ref={scrollAreaRef} className="flex-1 p-4">
        <div className="space-y-4">
          {messages.length === 0 ? (
            <div className="text-center text-muted-foreground py-8">
              Henüz mesaj yok. İlk mesajı gönderin!
            </div>
          ) : (
            messages.map((message) => {
              const isOwn = message.sender_id === currentUserId
              
              return (
                <div
                  key={message.id}
                  className={`flex ${isOwn ? 'justify-end' : 'justify-start'}`}
                >
                  <div
                    className={`max-w-[70%] rounded-lg px-4 py-2 ${
                      isOwn
                        ? 'bg-primary text-primary-foreground'
                        : 'bg-muted'
                    }`}
                  >
                    <p className="text-sm whitespace-pre-wrap break-words">
                      {message.content}
                    </p>
                    <p
                      className={`text-xs mt-1 ${
                        isOwn ? 'text-primary-foreground/70' : 'text-muted-foreground'
                      }`}
                    >
                      {formatTimestamp(message.created_at)}
                    </p>
                  </div>
                </div>
              )
            })
          )}
        </div>
      </ScrollArea>
      
      <CardContent className="border-t p-4">
        <form onSubmit={handleSendMessage} className="flex gap-2">
          <Input
            placeholder="Mesaj yazın..."
            value={newMessage}
            onChange={(e) => {
              setNewMessage(e.target.value)
              handleTyping()
            }}
            disabled={sending}
            className="flex-1"
          />
          <Button type="submit" disabled={sending || !newMessage.trim()}>
            <Send className="h-4 w-4" />
          </Button>
        </form>
      </CardContent>
    </div>
  )
}
