"use client"

import { useState, useEffect } from "react"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/app/components/ui/card"

interface FriendRequest {
  id: number
  requester_id: number
  addressee_id: number
  status: string
  created_at: string
  requester?: {
    id: number
    username: string
    name?: string
    avatar_url?: string
  }
}

export function FriendRequests() {
  const [requests, setRequests] = useState<FriendRequest[]>([])
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    fetchFriendRequests()
  }, [])

  const fetchFriendRequests = async () => {
    try {
      const response = await fetch('/api/friendships?status=pending', {
        headers: {
          'X-User-ID': '1', // TODO: Get from auth context
        },
      })
      if (response.ok) {
        const data = await response.json()
        setRequests(data)
      }
    } catch (error) {
      console.error('Error fetching friend requests:', error)
    } finally {
      setLoading(false)
    }
  }

  const handleAccept = async (requestId: number) => {
    try {
      const response = await fetch(`/api/friendships/${requestId}/accept`, {
        method: 'PUT',
        headers: {
          'X-User-ID': '1', // TODO: Get from auth context
        },
      })
      if (response.ok) {
        setRequests(requests.filter(r => r.id !== requestId))
      }
    } catch (error) {
      console.error('Error accepting friend request:', error)
    }
  }

  const handleReject = async (requestId: number) => {
    try {
      const response = await fetch(`/api/friendships/${requestId}/reject`, {
        method: 'PUT',
        headers: {
          'X-User-ID': '1', // TODO: Get from auth context
        },
      })
      if (response.ok) {
        setRequests(requests.filter(r => r.id !== requestId))
      }
    } catch (error) {
      console.error('Error rejecting friend request:', error)
    }
  }

  if (loading) {
    return <div>Loading...</div>
  }

  if (requests.length === 0) {
    return (
      <Card>
        <CardHeader>
          <CardTitle>Friend Requests</CardTitle>
          <CardDescription>No pending friend requests</CardDescription>
        </CardHeader>
      </Card>
    )
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>Friend Requests</CardTitle>
        <CardDescription>{requests.length} pending request(s)</CardDescription>
      </CardHeader>
      <CardContent className="space-y-4">
        {requests.map((request) => (
          <div key={request.id} className="flex items-center justify-between p-4 border rounded-lg">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-full bg-gray-200 flex items-center justify-center">
                {request.requester?.username?.[0]?.toUpperCase() || '?'}
              </div>
              <div>
                <p className="font-medium">{request.requester?.name || request.requester?.username}</p>
                <p className="text-sm text-gray-500">@{request.requester?.username}</p>
              </div>
            </div>
            <div className="flex gap-2">
              <Button onClick={() => handleAccept(request.id)} size="sm">
                Accept
              </Button>
              <Button onClick={() => handleReject(request.id)} variant="outline" size="sm">
                Decline
              </Button>
            </div>
          </div>
        ))}
      </CardContent>
    </Card>
  )
}
