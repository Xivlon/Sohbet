"use client"

import { useState, useEffect } from 'react'
import { Card } from '@/components/card'
import { ChatList } from '@/components/chat-list'
import { ChatWindow } from '@/components/chat-window'

export default function MessagesPage() {
  const [selectedConversationId, setSelectedConversationId] = useState<number | undefined>()
  const [currentUserId, setCurrentUserId] = useState<number>(1) // TODO: Get from auth context
  const [otherUser, setOtherUser] = useState<{ id: number; username: string } | undefined>()

  const handleSelectConversation = async (conversationId: number) => {
    setSelectedConversationId(conversationId)
    
    // Fetch conversation details to get other user info
    try {
      const response = await fetch('/api/conversations', {
        headers: {
          'X-User-ID': currentUserId.toString()
        }
      })
      
      if (response.ok) {
        const data = await response.json()
        const conversation = data.conversations.find((c: any) => c.id === conversationId)
        
        if (conversation) {
          const otherUserId = conversation.user1_id === currentUserId 
            ? conversation.user2_id 
            : conversation.user1_id
          
          const userResponse = await fetch(`/api/users/${otherUserId}`)
          if (userResponse.ok) {
            const userData = await userResponse.json()
            setOtherUser({
              id: otherUserId,
              username: userData.username || 'Unknown User'
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
