"use client"

import { useEffect, useState } from 'react'
import { Card, CardContent } from '@/app/components/ui/card'
import { Avatar, AvatarFallback } from '@/app/components/avatar'
import { ScrollArea } from '@/app/components/ui/scroll-area'
import { useOnlineUsers } from '../lib/use-websocket'

interface Conversation {
  id: number
  user1_id: number
  user2_id: number
  created_at: string
  last_message_at: string
  other_user?: {
    id: number
    username: string
    email: string
  }
}

interface ChatListProps {
  currentUserId: number
  onSelectConversation: (conversationId: number) => void
  selectedConversationId?: number
}

export function ChatList({ currentUserId, onSelectConversation, selectedConversationId }: ChatListProps) {
  const [conversations, setConversations] = useState<Conversation[]>([])
  const [loading, setLoading] = useState(true)
  
  // Use WebSocket hook to track online users
  const { isOnline } = useOnlineUsers()

  useEffect(() => {
    fetchConversations()
  }, [])

  const fetchConversations = async () => {
    try {
      const response = await fetch('/api/conversations', {
        headers: {
          'X-User-ID': currentUserId.toString()
        }
      })
      
      if (response.ok) {
        const data = await response.json()
        
        // Fetch user details for each conversation
        const conversationsWithUsers = await Promise.all(
          data.conversations.map(async (conv: Conversation) => {
            const otherUserId = conv.user1_id === currentUserId ? conv.user2_id : conv.user1_id
            
            try {
              const userResponse = await fetch(`/api/users/${otherUserId}`)
              if (userResponse.ok) {
                const userData = await userResponse.json()
                return {
                  ...conv,
                  other_user: {
                    id: otherUserId,
                    username: userData.username || 'Unknown User',
                    email: userData.email || ''
                  }
                }
              }
            } catch (error) {
              console.error('Error fetching user:', error)
            }
            
            return {
              ...conv,
              other_user: {
                id: otherUserId,
                username: 'Unknown User',
                email: ''
              }
            }
          })
        )
        
        setConversations(conversationsWithUsers)
      }
    } catch (error) {
      console.error('Error fetching conversations:', error)
    } finally {
      setLoading(false)
    }
  }

  if (loading) {
    return (
      <div className="p-4 text-center text-muted-foreground">
        Yükleniyor...
      </div>
    )
  }

  if (conversations.length === 0) {
    return (
      <div className="p-4 text-center text-muted-foreground">
        Henüz mesajınız yok
      </div>
    )
  }

  return (
    <ScrollArea className="h-full">
      <div className="space-y-2 p-4">
        {conversations.map((conversation) => {
          const userIsOnline = conversation.other_user ? isOnline(conversation.other_user.id) : false
          
          return (
            <Card
              key={conversation.id}
              className={`cursor-pointer transition-colors hover:bg-accent ${
                selectedConversationId === conversation.id ? 'bg-accent' : ''
              }`}
              onClick={() => onSelectConversation(conversation.id)}
            >
              <CardContent className="p-4 flex items-center gap-3">
                <div className="relative">
                  <Avatar>
                    <AvatarFallback>
                      {conversation.other_user?.username?.charAt(0).toUpperCase() || 'U'}
                    </AvatarFallback>
                  </Avatar>
                  {userIsOnline && (
                    <div className="absolute bottom-0 right-0 w-3 h-3 bg-green-500 rounded-full border-2 border-background" />
                  )}
                </div>
                <div className="flex-1 min-w-0">
                  <p className="font-medium truncate">
                    {conversation.other_user?.username || 'Unknown User'}
                  </p>
                  <p className="text-sm text-muted-foreground truncate">
                    {new Date(conversation.last_message_at).toLocaleDateString('tr-TR')}
                  </p>
                </div>
              </CardContent>
            </Card>
          )
        })}
      </div>
    </ScrollArea>
  )
}
