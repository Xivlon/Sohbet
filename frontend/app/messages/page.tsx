"use client"

import { useState } from 'react'
import { Card } from '@/app/components/ui/card'
import { ChatList } from '@/app/components/chat-list'
import { ChatWindow } from '@/app/components/chat-window'
import { apiClient } from '@/app/lib/api-client'

interface Conversation {
  id: number
  user1_id: number
  user2_id: number
  created_at: string
}

export default function MessagesPage() {
  const [selectedConversationId, setSelectedConversationId] = useState<number | undefined>()
  const [currentUserId] = useState<number>(1) // TODO: Get from auth context
  const [otherUser, setOtherUser] = useState<{ id: number; username: string } | undefined>()

  const handleSelectConversation = async (conversationId: number) => {
    setSelectedConversationId(conversationId)
    
    // Fetch conversation details to get other user info
    try {
      const response = await apiClient.getConversations()
      
      if (response.data) {
        const conversation: Conversation | undefined = response.data.conversations.find((c: Conversation) => c.id === conversationId)
        
        if (conversation) {
          const otherUserId = conversation.user1_id === currentUserId 
            ? conversation.user2_id 
            : conversation.user1_id
          
          const userResponse = await apiClient.getUserById(otherUserId)
          if (userResponse.data) {
            setOtherUser({
              id: otherUserId,
              username: userResponse.data.username || 'Unknown User'
            })
          }
        }
      }
    } catch (error) {
      console.error('Error fetching conversation details:', error)
    }
  }

  return (
    <div className="container mx-auto p-4 h-[calc(100vh-4rem)]">
      <h1 className="text-3xl font-bold mb-4">Mesajlar</h1>
      
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 h-[calc(100%-4rem)]">
        {/* Conversations List */}
        <Card className="md:col-span-1 h-full overflow-hidden">
          <ChatList
            currentUserId={currentUserId}
            onSelectConversation={handleSelectConversation}
            selectedConversationId={selectedConversationId}
          />
        </Card>
        
        {/* Chat Window */}
        <Card className="md:col-span-2 h-full overflow-hidden">
          {selectedConversationId && otherUser ? (
            <ChatWindow
              conversationId={selectedConversationId}
              currentUserId={currentUserId}
              otherUser={otherUser}
            />
          ) : (
            <div className="h-full flex items-center justify-center text-muted-foreground">
              Bir sohbet seçin veya yeni sohbet başlatın
            </div>
          )}
        </Card>
      </div>
    </div>
  )
}
