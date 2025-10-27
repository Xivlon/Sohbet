"use client"

import { useEffect, useState } from "react"
import { GroupCard } from "./group-card"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/app/components/ui/tabs"
import { Skeleton } from "@/app/components/ui/skeleton"

interface Group {
  id: number
  name: string
  description?: string
  creator_id: number
  privacy: string
  created_at: string
  member_count?: number
  user_role?: string
}

interface GroupListProps {
  currentUserId?: number
  onGroupSelect?: (groupId: number) => void
}

export function GroupList({ currentUserId, onGroupSelect }: GroupListProps) {
  const [allGroups, setAllGroups] = useState<Group[]>([])
  const [myGroups, setMyGroups] = useState<Group[]>([])
  const [loading, setLoading] = useState(true)
  const [activeTab, setActiveTab] = useState("all")

  useEffect(() => {
    fetchGroups()
  }, [currentUserId])

  const fetchGroups = async () => {
    if (!currentUserId) return

    setLoading(true)
    try {
      // Fetch all groups
      const allResponse = await fetch('/api/groups', {
        headers: {
          'X-User-ID': String(currentUserId),
        },
      })
      if (allResponse.ok) {
        const data = await allResponse.json()
        setAllGroups(data)
      }

      // Fetch my groups
      const myResponse = await fetch('/api/groups?my_groups=true', {
        headers: {
          'X-User-ID': String(currentUserId),
        },
      })
      if (myResponse.ok) {
        const data = await myResponse.json()
        setMyGroups(data)
      }
    } catch (error) {
      console.error('Error fetching groups:', error)
    } finally {
      setLoading(false)
    }
  }

  const handleJoin = async (groupId: number) => {
    if (!currentUserId) return

    try {
      const response = await fetch(`/api/groups/${groupId}/members`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'X-User-ID': String(currentUserId),
        },
        body: JSON.stringify({ user_id: currentUserId, role: 'member' }),
      })

      if (response.ok) {
        // Refresh groups
        await fetchGroups()
      } else {
        const error = await response.json()
        alert(error.error || 'Failed to join group')
      }
    } catch (error) {
      console.error('Error joining group:', error)
      alert('Failed to join group')
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

  return (
    <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
      <TabsList className="grid w-full max-w-md grid-cols-2">
        <TabsTrigger value="all">All Groups</TabsTrigger>
        <TabsTrigger value="my">My Groups</TabsTrigger>
      </TabsList>
      <TabsContent value="all" className="mt-6">
        {allGroups.length === 0 ? (
          <div className="text-center py-12 text-muted-foreground">
            No groups available yet.
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {allGroups.map((group) => (
              <GroupCard
                key={group.id}
                group={group}
                currentUserId={currentUserId}
                onJoin={handleJoin}
                onManage={handleManage}
                onView={handleView}
              />
            ))}
          </div>
        )}
      </TabsContent>
      <TabsContent value="my" className="mt-6">
        {myGroups.length === 0 ? (
          <div className="text-center py-12 text-muted-foreground">
            You haven't joined any groups yet.
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {myGroups.map((group) => (
              <GroupCard
                key={group.id}
                group={group}
                currentUserId={currentUserId}
                onJoin={handleJoin}
                onManage={handleManage}
                onView={handleView}
              />
            ))}
          </div>
        )}
      </TabsContent>
    </Tabs>
  )
}
