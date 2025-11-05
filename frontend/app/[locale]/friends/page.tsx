"use client"

import { useState, useEffect } from "react"
import { useRouter } from "next/navigation"
import { ArrowLeft } from "lucide-react"
import { FriendRequests } from "@/app/components/friend-requests"
import { FriendsList } from "@/app/components/friends-list"
import { Header } from "@/app/components/header"
import { Button } from "@/app/components/ui/button"

export default function FriendsPage() {
  const router = useRouter()
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

  return (
    <div className="flex flex-col min-h-screen bg-background">
      <Header isDarkMode={isDarkMode} onToggleDarkMode={toggleDarkMode} />
      <div className="container mx-auto py-8 px-4">
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
        <h1 className="text-3xl font-bold mb-8">Friends</h1>
        
        <div className="grid gap-6 md:grid-cols-1 lg:grid-cols-2">
          <FriendRequests />
          <FriendsList />
        </div>
      </div>
    </div>
  )
}
