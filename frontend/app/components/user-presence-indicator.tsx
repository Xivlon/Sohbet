"use client"

import React, { useEffect, useState } from 'react'
import { cn } from '../lib/utils'

interface UserPresenceIndicatorProps {
  userId: number
  showStatus?: boolean
  className?: string
}

type PresenceStatus = 'online' | 'away' | 'busy' | 'offline'

interface UserPresence {
  status: PresenceStatus
  custom_status?: string
  last_seen?: string
}

export function UserPresenceIndicator({
  userId,
  showStatus = false,
  className
}: UserPresenceIndicatorProps) {
  const [presence, setPresence] = useState<UserPresence>({ status: 'offline' })

  useEffect(() => {
    // Fetch presence status
    fetchPresence()

    // Poll for updates every 60 seconds
    const interval = setInterval(fetchPresence, 60000)
    return () => clearInterval(interval)
  }, [userId])

  const fetchPresence = async () => {
    try {
      const response = await fetch(`/api/users/${userId}/presence`, {
        credentials: 'include'
      })
      if (response.ok) {
        const data = await response.json()
        setPresence(data)
      }
    } catch (error) {
      console.error('Failed to fetch presence:', error)
    }
  }

  const getStatusColor = () => {
    switch (presence.status) {
      case 'online':
        return 'bg-green-500'
      case 'away':
        return 'bg-yellow-500'
      case 'busy':
        return 'bg-red-500'
      default:
        return 'bg-gray-400'
    }
  }

  const getStatusText = () => {
    switch (presence.status) {
      case 'online':
        return 'Online'
      case 'away':
        return 'Away'
      case 'busy':
        return 'Busy'
      default:
        return 'Offline'
    }
  }

  return (
    <div className={cn("flex items-center gap-2", className)}>
      <div className={cn(
        "h-3 w-3 rounded-full ring-2 ring-background",
        getStatusColor()
      )} />
      {showStatus && (
        <div>
          <span className="text-sm font-medium">{getStatusText()}</span>
          {presence.custom_status && (
            <p className="text-xs text-muted-foreground">{presence.custom_status}</p>
          )}
        </div>
      )}
    </div>
  )
}
