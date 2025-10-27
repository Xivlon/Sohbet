"use client"

import { useState, useEffect } from "react"
import { GroupList } from "@/app/components/group-list"
import { GroupCreator } from "@/app/components/group-creator"
import { useAuth } from "@/app/contexts/auth-context"

export default function GroupsPage() {
  const { user } = useAuth()
  const [refreshKey, setRefreshKey] = useState(0)

  const handleGroupCreated = () => {
    // Refresh the group list
    setRefreshKey(prev => prev + 1)
  }

  return (
    <div className="container mx-auto py-8 px-4">
      <div className="flex justify-between items-center mb-8">
        <div>
          <h1 className="text-3xl font-bold">Groups</h1>
          <p className="text-muted-foreground mt-2">
            Join academic groups to collaborate and discuss with peers
          </p>
        </div>
        <GroupCreator
          currentUserId={user?.id}
          onGroupCreated={handleGroupCreated}
        />
      </div>
      <GroupList key={refreshKey} currentUserId={user?.id} />
    </div>
  )
}
