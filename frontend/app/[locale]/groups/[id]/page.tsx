"use client"

import { useEffect, useState } from "react"
import { useParams } from "next/navigation"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/app/components/ui/tabs"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/app/components/ui/card"
import { Button } from "@/app/components/ui/button"
import { Skeleton } from "@/app/components/ui/skeleton"
import { ArrowLeft, Users, Lock, Globe, Home } from "lucide-react"
import { Badge } from "@/app/components/ui/badge"
import { Header } from "@/app/components/header"
import { GroupAnnouncements } from "@/app/components/group-announcements"
import { GroupCloudAccess } from "@/app/components/group-cloud-access"
import { apiClient, Group } from "@/app/lib/api-client"
import { useAuth } from "@/app/contexts/auth-context"
import Link from "next/link"

export default function GroupDetailPage() {
  const params = useParams()
  const groupId = parseInt(params.id as string)
  const locale = params.locale as string
  const { user } = useAuth()
  const [group, setGroup] = useState<Group | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
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

  useEffect(() => {
    if (groupId) {
      fetchGroupDetails()
    }
  }, [groupId])

  const fetchGroupDetails = async () => {
    setLoading(true)
    setError(null)

    try {
      // For now, we'll use mock data since we need to implement the detail endpoint
      // In production, you would call: await apiClient.getGroup(groupId)

      // Mock data
      const mockGroups: Group[] = [
        {
          id: 1,
          name: "Computer Science 101",
          description: "Introduction to Computer Science - Fall 2024",
          privacy: "public",
          created_by: 1,
          created_at: new Date().toISOString(),
          member_count: 45,
          user_role: "member"
        },
        {
          id: 2,
          name: "Data Structures Study Group",
          description: "Collaborative learning for Data Structures and Algorithms",
          privacy: "private",
          created_by: 2,
          created_at: new Date().toISOString(),
          member_count: 28,
          user_role: "member"
        },
        {
          id: 3,
          name: "Machine Learning Research",
          description: "Advanced research group for ML projects and discussions",
          privacy: "invite_only",
          created_by: 1,
          created_at: new Date().toISOString(),
          member_count: 15,
          user_role: "admin"
        }
      ]

      const foundGroup = mockGroups.find(g => g.id === groupId)
      if (foundGroup) {
        setGroup(foundGroup)
      } else {
        setError("Group not found")
      }
    } catch (err) {
      setError('Failed to load group details')
      console.error('Error fetching group:', err)
    } finally {
      setLoading(false)
    }
  }

  const getPrivacyIcon = () => {
    switch (group?.privacy) {
      case 'public':
        return <Globe className="h-4 w-4" />
      case 'private':
        return <Lock className="h-4 w-4" />
      case 'invite_only':
        return <Lock className="h-4 w-4" />
      default:
        return <Lock className="h-4 w-4" />
    }
  }

  const getPrivacyLabel = () => {
    switch (group?.privacy) {
      case 'public':
        return 'Public'
      case 'private':
        return 'Private'
      case 'invite_only':
        return 'Invite Only'
      default:
        return 'Private'
    }
  }

  if (loading) {
    return (
      <div className="flex flex-col min-h-screen bg-background">
        <Header isDarkMode={isDarkMode} onToggleDarkMode={toggleDarkMode} />
        <div className="container mx-auto py-8 px-4">
          <Skeleton className="h-8 w-64 mb-4" />
          <Skeleton className="h-24 w-full mb-6" />
          <Skeleton className="h-96 w-full" />
        </div>
      </div>
    )
  }

  if (error || !group) {
    return (
      <div className="flex flex-col min-h-screen bg-background">
        <Header isDarkMode={isDarkMode} onToggleDarkMode={toggleDarkMode} />
        <div className="container mx-auto py-8 px-4">
          <Card>
            <CardContent className="p-8 text-center">
              <p className="text-destructive mb-4">{error || "Group not found"}</p>
              <div className="flex gap-2 justify-center">
                <Button asChild variant="outline">
                  <Link href={`/${locale}`}>
                    <Home className="h-4 w-4 mr-2" />
                    Home
                  </Link>
                </Button>
                <Button asChild>
                  <Link href={`/${locale}/groups`}>
                    <ArrowLeft className="h-4 w-4 mr-2" />
                    Back to Groups
                  </Link>
                </Button>
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    )
  }

  return (
    <div className="flex flex-col min-h-screen bg-background">
      <Header isDarkMode={isDarkMode} onToggleDarkMode={toggleDarkMode} />
      <div className="container mx-auto py-8 px-4">
        <div className="mb-6">
          <div className="flex gap-2 mb-4">
            <Button variant="ghost" asChild>
              <Link href={`/${locale}`}>
                <Home className="h-4 w-4 mr-2" />
                Home
              </Link>
            </Button>
            <Button variant="ghost" asChild>
              <Link href={`/${locale}/groups`}>
                <ArrowLeft className="h-4 w-4 mr-2" />
                Back to Groups
              </Link>
            </Button>
          </div>

          <Card>
            <CardHeader>
              <div className="flex items-start justify-between">
                <div className="flex-1">
                  <CardTitle className="text-3xl">{group.name}</CardTitle>
                  <CardDescription className="mt-2 text-base">
                    {group.description}
                  </CardDescription>
                  <div className="flex items-center gap-2 mt-4">
                    <Badge variant="secondary" className="flex items-center gap-1">
                      {getPrivacyIcon()}
                      <span>{getPrivacyLabel()}</span>
                    </Badge>
                    {group.member_count !== undefined && (
                      <Badge variant="outline" className="flex items-center gap-1">
                        <Users className="h-3 w-3" />
                        <span>{group.member_count} members</span>
                      </Badge>
                    )}
                    {group.user_role && (
                      <Badge variant="default" className="capitalize">
                        {group.user_role}
                      </Badge>
                    )}
                  </div>
                </div>
              </div>
            </CardHeader>
          </Card>
        </div>

        <Tabs defaultValue="announcements" className="w-full">
          <TabsList className="grid w-full max-w-md grid-cols-2">
            <TabsTrigger value="announcements">Announcements</TabsTrigger>
            <TabsTrigger value="cloud">Cloud Access</TabsTrigger>
          </TabsList>

          <TabsContent value="announcements" className="mt-6">
            <GroupAnnouncements groupId={groupId} userRole={group.user_role} />
          </TabsContent>

          <TabsContent value="cloud" className="mt-6">
            <GroupCloudAccess groupId={groupId} userRole={group.user_role} />
          </TabsContent>
        </Tabs>
      </div>
    </div>
  )
}
