"use client"

import { useState } from "react"
import { Button } from "@/app/components/ui/button"
import { Card, CardDescription, CardFooter, CardHeader, CardTitle } from "@/app/components/ui/card"
import { Users, Lock, Globe, Settings } from "lucide-react"
import { Badge } from "@/app/components/ui/badge"

// Privacy constants
const PRIVACY = {
  PUBLIC: 'public',
  PRIVATE: 'private',
  INVITE_ONLY: 'invite_only',
} as const;

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

interface GroupCardProps {
  group: Group
  currentUserId?: number
  onJoin?: (groupId: number) => void
  onManage?: (groupId: number) => void
  onView?: (groupId: number) => void
}

export function GroupCard({ group, currentUserId, onJoin, onManage, onView }: GroupCardProps) {
  const [isJoining, setIsJoining] = useState(false)
  const isMember = group.user_role !== undefined && group.user_role !== null
  const canManage = group.user_role === 'admin' || group.user_role === 'moderator' || group.creator_id === currentUserId

  const handleJoin = async () => {
    if (!onJoin || !currentUserId) return
    
    setIsJoining(true)
    try {
      await onJoin(group.id)
    } finally {
      setIsJoining(false)
    }
  }

  const getPrivacyIcon = () => {
    switch (group.privacy) {
      case PRIVACY.PUBLIC:
        return <Globe className="h-4 w-4" />
      case PRIVACY.PRIVATE:
        return <Lock className="h-4 w-4" />
      case PRIVACY.INVITE_ONLY:
        return <Lock className="h-4 w-4" />
      default:
        return <Lock className="h-4 w-4" />
    }
  }

  const getPrivacyLabel = () => {
    switch (group.privacy) {
      case PRIVACY.PUBLIC:
        return 'Public'
      case PRIVACY.PRIVATE:
        return 'Private'
      case PRIVACY.INVITE_ONLY:
        return 'Invite Only'
      default:
        return 'Private'
    }
  }

  return (
    <Card className="hover:shadow-lg transition-shadow">
      <CardHeader>
        <div className="flex items-start justify-between">
          <div className="flex-1">
            <CardTitle className="text-xl">{group.name}</CardTitle>
            <div className="flex items-center gap-2 mt-2">
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
            </div>
          </div>
          {canManage && (
            <Button
              variant="ghost"
              size="icon"
              onClick={() => onManage?.(group.id)}
            >
              <Settings className="h-4 w-4" />
            </Button>
          )}
        </div>
        {group.description && (
          <CardDescription className="mt-2">{group.description}</CardDescription>
        )}
      </CardHeader>
      <CardFooter className="flex gap-2">
        {!isMember ? (
          <Button
            onClick={handleJoin}
            disabled={isJoining || group.privacy === PRIVACY.INVITE_ONLY}
            className="w-full"
          >
            {isJoining ? 'Joining...' : group.privacy === PRIVACY.INVITE_ONLY ? 'Invite Required' : 'Join Group'}
          </Button>
        ) : (
          <Button
            variant="outline"
            onClick={() => onView?.(group.id)}
            className="w-full"
          >
            View Group
          </Button>
        )}
      </CardFooter>
    </Card>
  )
}
