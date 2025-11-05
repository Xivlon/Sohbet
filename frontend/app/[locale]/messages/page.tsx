"use client"

import { useState, useEffect } from 'react'
import { useRouter } from "next/navigation"
import { ArrowLeft } from "lucide-react"
import { Card } from '@/app/components/ui/card'
import { ChatList } from '@/app/components/chat-list'
import { ChatWindow } from '@/app/components/chat-window'
import { Header } from "@/app/components/header"
import { Button } from "@/app/components/ui/button"
import { apiClient } from '@/app/lib/api-client'

interface Conversation {
  id: number
  user1_id: number
  user2_id: number
  created_at: string
}

export default function MessagesPage() {
  const router = useRouter()
  const [selectedConversationId, setSelectedConversationId] = useState<number | undefined>()
  const [currentUserId] = useState<number>(1) // TODO: Get from auth context
  const [otherUser, setOtherUser] = useState<{ id: number; username: string } | undefined>()
  const [isDarkMode, setIsDarkMode] = useState(false)

  useEffect(() => {
    // Check for dark mode preference
    if (typeof window !== 'undefined') {
      const stored = localStorage.getItem('darkMode')
      if (stored !== null) {
        setIsDarkMode(stored === 'true')
      } else {
        setIsDarkMode(window.matchMedia('(prefers-color-scheme: dark)').matches)
      }
    }
  }, [])

  useEffect(() => {
    // Apply dark mode class
    if (isDarkMode) {
      document.documentElement.classList.add('dark')
    } else {
      document.documentElement.classList.remove('dark')
    }
    localStorage.setItem('darkMode', String(isDarkMode))
  }, [isDarkMode])

  const toggleDarkMode = () => {
    setIsDarkMode(!isDarkMode)
  }

  const handleBackToHome = () => {
    router.push('/')
  }

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
    <div className="flex flex-col min-h-screen bg-background">
      <Header isDarkMode={isDarkMode} onToggleDarkMode={toggleDarkMode} />
      <div className="container mx-auto p-4 flex-1 flex flex-col">
        <div className="mb-6">
          <Button
            variant="ghost"
            size="sm"
            onClick={handleBackToHome}
            className="flex items-center gap-2 mb-4"
          >
            <ArrowLeft className="w-4 h-4" />
            Back to Home
          </Button>
        </div>
        <h1 className="text-3xl font-bold mb-4">Mesajlar</h1>
        
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4 flex-1 min-h-0">
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
    </div>
  )
}
