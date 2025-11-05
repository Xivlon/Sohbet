"use client"

import { useState, useEffect } from "react"
import { useRouter } from "next/navigation"
import { ArrowLeft } from "lucide-react"
import { GroupList } from "@/app/components/group-list"
import { GroupCreator } from "@/app/components/group-creator"
import { Header } from "@/app/components/header"
import { Button } from "@/app/components/ui/button"
import { useAuth } from "@/app/contexts/auth-context"

export default function GroupsPage() {
  const { user } = useAuth()
  const router = useRouter()
  const [refreshKey, setRefreshKey] = useState(0)
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

  const handleGroupCreated = () => {
    // Refresh the group list
    setRefreshKey(prev => prev + 1)
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
        <div className="flex justify-between items-center mb-8">
          <div>
            <h1 className="text-3xl font-bold">Groups</h1>
            <p className="text-muted-foreground mt-2">
              Join academic groups to collaborate and discuss with peers
            </p>
          </div>
          <GroupCreator
            onGroupCreated={handleGroupCreated}
          />
        </div>
        <GroupList key={refreshKey} currentUserId={user?.id} />
      </div>
    </div>
  )
}
