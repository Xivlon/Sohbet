"use client"

import { useEffect, useState } from "react"
import { useTranslations } from "next-intl"
import { GroupCard } from "./group-card"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/app/components/ui/tabs"
import { Skeleton } from "@/app/components/ui/skeleton"
import { apiClient, Group } from "@/app/lib/api-client"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent } from "@/app/components/ui/card"
import { ArrowLeft } from "lucide-react"

interface GroupListProps {
  currentUserId?: number
  onGroupSelect?: (groupId: number) => void
  onLeave?: () => void
}

export function GroupList({ currentUserId, onGroupSelect, onLeave }: GroupListProps) {
  const t = useTranslations('groups')
  const tCommon = useTranslations('common')
  const tSearch = useTranslations('search')
  const tErrors = useTranslations('errors')
  const tSidebar = useTranslations('sidebar')

  const [allGroups, setAllGroups] = useState<Group[]>([])
  const [myGroups, setMyGroups] = useState<Group[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [activeTab, setActiveTab] = useState("all")

  useEffect(() => {
    fetchGroups()
  }, [currentUserId])

  const fetchGroups = async () => {
    if (!currentUserId) {
      setLoading(false);
      return;
    }

    setLoading(true)
    setError(null)

    // Mock groups for demonstration
    const mockGroups: Group[] = [
      {
        id: 1,
        name: "Computer Science 101",
        description: "Introduction to Computer Science - Fall 2024. Learn the fundamentals of programming, algorithms, and data structures.",
        privacy: "public",
        created_by: 1,
        created_at: new Date(Date.now() - 30 * 24 * 60 * 60 * 1000).toISOString(),
        member_count: 45,
        user_role: "member"
      },
      {
        id: 2,
        name: "Data Structures Study Group",
        description: "Collaborative learning for Data Structures and Algorithms. Weekly problem-solving sessions and code reviews.",
        privacy: "private",
        created_by: 2,
        created_at: new Date(Date.now() - 20 * 24 * 60 * 60 * 1000).toISOString(),
        member_count: 28,
        user_role: "member"
      },
      {
        id: 3,
        name: "Machine Learning Research",
        description: "Advanced research group for ML projects and discussions. Focus on deep learning, NLP, and computer vision.",
        privacy: "invite_only",
        created_by: 1,
        created_at: new Date(Date.now() - 15 * 24 * 60 * 60 * 1000).toISOString(),
        member_count: 15,
        user_role: "admin"
      },
      {
        id: 4,
        name: "Web Development Workshop",
        description: "Hands-on web development with React, Next.js, and modern frameworks. Build real-world projects together.",
        privacy: "public",
        created_by: 3,
        created_at: new Date(Date.now() - 10 * 24 * 60 * 60 * 1000).toISOString(),
        member_count: 62
      },
      {
        id: 5,
        name: "Algorithms Competition Team",
        description: "Prepare for coding competitions and algorithmic challenges. Practice with LeetCode, Codeforces, and more.",
        privacy: "private",
        created_by: 2,
        created_at: new Date(Date.now() - 8 * 24 * 60 * 60 * 1000).toISOString(),
        member_count: 18
      },
      {
        id: 6,
        name: "Database Systems",
        description: "Study group for database design, SQL, NoSQL, and database optimization. Includes hands-on projects.",
        privacy: "public",
        created_by: 4,
        created_at: new Date(Date.now() - 5 * 24 * 60 * 60 * 1000).toISOString(),
        member_count: 34,
        user_role: "moderator"
      }
    ]

    try {
      const response = await apiClient.getGroups(50, 0)
      if (response.data) {
        const apiGroups = response.data.groups || []
        // Combine API groups with mock groups, preferring API groups
        const combinedGroups = apiGroups.length > 0 ? apiGroups : mockGroups
        setAllGroups(combinedGroups)

        // Filter my groups (where user is a member)
        const myGroupsList = combinedGroups.filter((g: Group) => g.user_role)
        setMyGroups(myGroupsList)
      } else {
        // Use mock groups if API fails
        setAllGroups(mockGroups)
        const myGroupsList = mockGroups.filter((g: Group) => g.user_role)
        setMyGroups(myGroupsList)
      }
    } catch (err) {
      // Use mock groups on error
      console.log('Using mock groups due to API error:', err)
      setAllGroups(mockGroups)
      const myGroupsList = mockGroups.filter((g: Group) => g.user_role)
      setMyGroups(myGroupsList)
    } finally {
      setLoading(false)
    }
  }

  const handleJoin = async (groupId: number) => {
    if (!currentUserId) return

    try {
      const response = await apiClient.joinGroup(groupId, currentUserId)
      if (response.data) {
        // Refresh groups
        await fetchGroups()
      } else {
        alert(`${tErrors('somethingWentWrong')}: ${response.error || tErrors('somethingWentWrong')}`)
      }
    } catch (error) {
      console.error('Error joining group:', error)
      alert(`${tErrors('somethingWentWrong')}. ${tErrors('tryAgain')}.`)
    }
  }

  const handleManage = (groupId: number) => {
    // Navigate to group management page
    window.location.href = `/groups/${groupId}/manage`
  }

  const handleView = (groupId: number) => {
    if (onGroupSelect) {
      onGroupSelect(groupId)
    } else {
      window.location.href = `/groups/${groupId}`
    }
  }

  if (loading) {
    return (
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {[1, 2, 3, 4, 5, 6].map((i) => (
          <Skeleton key={i} className="h-48 w-full" />
        ))}
      </div>
    )
  }

  if (error) {
    return (
      <Card>
        <CardContent className="p-8 text-center">
          <p className="text-destructive mb-4">{error}</p>
          <Button onClick={fetchGroups}>{tErrors('tryAgain')}</Button>
        </CardContent>
      </Card>
    )
  }

  return (
    <div className="w-full space-y-4">
      {onLeave && (
        <div className="flex items-center gap-4">
          <Button
            variant="ghost"
            size="sm"
            onClick={onLeave}
            className="flex items-center gap-2"
          >
            <ArrowLeft className="w-4 h-4" />
            {tCommon('back')} {tSidebar('home')}
          </Button>
        </div>
      )}

      <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
        <TabsList className="grid w-full max-w-md grid-cols-2">
          <TabsTrigger value="all">{t('discoverGroups')}</TabsTrigger>
          <TabsTrigger value="my">{t('myGroups')}</TabsTrigger>
        </TabsList>
      
      <TabsContent value="all" className="mt-6">
        {allGroups.length === 0 ? (
          <Card>
            <CardContent className="p-8 text-center">
              <p className="text-muted-foreground">{tSearch('noResults')}</p>
            </CardContent>
          </Card>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {allGroups.map((group) => (
              <GroupCard
                key={group.id}
                group={group}
                onJoin={() => handleJoin(group.id)}
                onManage={() => handleManage(group.id)}
                onView={() => handleView(group.id)}
                currentUserId={currentUserId}
              />
            ))}
          </div>
        )}
      </TabsContent>
      
      <TabsContent value="my" className="mt-6">
        {myGroups.length === 0 ? (
          <Card>
            <CardContent className="p-8 text-center">
              <p className="text-muted-foreground">{tSearch('noResults')}</p>
            </CardContent>
          </Card>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {myGroups.map((group) => (
              <GroupCard
                key={group.id}
                group={group}
                onJoin={() => handleJoin(group.id)}
                onManage={() => handleManage(group.id)}
                onView={() => handleView(group.id)}
                currentUserId={currentUserId}
              />
            ))}
          </div>
        )}
      </TabsContent>
    </Tabs>
    </div>
  )
}
