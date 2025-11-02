"use client"

import { useEffect, useState } from "react"
import { GroupCard } from "./group-card"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/app/components/ui/tabs"
import { Skeleton } from "@/app/components/ui/skeleton"
import { apiClient, Group } from "@/app/lib/api-client"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent } from "@/app/components/ui/card"

interface GroupListProps {
  currentUserId?: number
  onGroupSelect?: (groupId: number) => void
}

export function GroupList({ currentUserId, onGroupSelect }: GroupListProps) {
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
    
    try {
      const response = await apiClient.getGroups(50, 0)
      if (response.data) {
        const groups = response.data.groups || []
        setAllGroups(groups)
        
        // Filter my groups (where user is a member)
        const myGroupsList = groups.filter((g: Group) => g.user_role)
        setMyGroups(myGroupsList)
      } else {
        setError(response.error || 'Gruplar yüklenemedi')
      }
    } catch (err) {
      setError('Bir hata oluştu. Lütfen tekrar deneyin.')
      console.error('Error fetching groups:', err)
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
        alert('Gruba katılma başarısız: ' + (response.error || 'Bilinmeyen hata'))
      }
    } catch (error) {
      console.error('Error joining group:', error)
      alert('Gruba katılma başarısız. Lütfen tekrar deneyin.')
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
          <Button onClick={fetchGroups}>Tekrar Dene</Button>
        </CardContent>
      </Card>
    )
  }

  return (
    <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
      <TabsList className="grid w-full max-w-md grid-cols-2">
        <TabsTrigger value="all">Tüm Gruplar</TabsTrigger>
        <TabsTrigger value="my">Gruplarım</TabsTrigger>
      </TabsList>
      
      <TabsContent value="all" className="mt-6">
        {allGroups.length === 0 ? (
          <Card>
            <CardContent className="p-8 text-center">
              <p className="text-muted-foreground">Henüz grup yok.</p>
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
              <p className="text-muted-foreground">Henüz hiçbir gruba katılmadınız.</p>
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
  )
}
