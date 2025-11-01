"use client"

import { useState, useEffect } from "react"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/app/components/ui/card"
import { apiClient } from '@/app/lib/api-client'
import { useAuth } from '@/app/contexts/auth-context'

interface Friend {
  id: number
  username: string
  name?: string
  email: string
  avatar_url?: string
  university?: string
  department?: string
}

interface Friendship {
  id: number
  requester_id: number
  addressee_id: number
  status: string
  created_at: string
  updated_at: string
}

export function FriendsList() {
  const [friends, setFriends] = useState<Friend[]>([])
  const [loading, setLoading] = useState(true)
  
  const { user } = useAuth()

  useEffect(() => {
    if (user?.id) {
      fetchFriends()
    }
  }, [user?.id])

  const fetchFriends = async () => {
    if (!user?.id) return
    
    try {
      const response = await apiClient.getFriends(user.id)
      if (response.data) {
        setFriends(response.data)
      }
    } catch (error) {
      console.error('Error fetching friends:', error)
    } finally {
      setLoading(false)
    }
  }

  const handleUnfriend = async (friendId: number) => {
    if (!confirm('Are you sure you want to remove this friend?')) return
    if (!user?.id) return

    try {
      // First find the friendship record
      const response = await apiClient.getFriendRequests()
      
      if (response.data) {
        const friendships: Friendship[] = response.data
        const friendship = friendships.find(
          (f) => 
            (f.requester_id === user.id && f.addressee_id === friendId) ||
            (f.requester_id === friendId && f.addressee_id === user.id)
        )
        
        if (friendship) {
          const deleteResponse = await apiClient.deleteFriendship(friendship.id)
          
          if (deleteResponse.status === 200 || deleteResponse.status === 204) {
            setFriends(friends.filter(f => f.id !== friendId))
          }
        }
      }
    } catch (error) {
      console.error('Error unfriending:', error)
    }
  }

  if (loading) {
    return <div>Loading...</div>
  }

  if (friends.length === 0) {
    return (
      <Card>
        <CardHeader>
          <CardTitle>Friends</CardTitle>
          <CardDescription>No friends yet</CardDescription>
        </CardHeader>
      </Card>
    )
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>Friends</CardTitle>
        <CardDescription>{friends.length} friend(s)</CardDescription>
      </CardHeader>
      <CardContent>
        <div className="grid gap-4">
          {friends.map((friend) => (
            <div key={friend.id} className="flex items-center justify-between p-4 border rounded-lg">
              <div className="flex items-center gap-3">
                <div className="w-12 h-12 rounded-full bg-gray-200 flex items-center justify-center">
                  {friend.username[0].toUpperCase()}
                </div>
                <div>
                  <p className="font-medium">{friend.name || friend.username}</p>
                  <p className="text-sm text-gray-500">@{friend.username}</p>
                  {friend.university && (
                    <p className="text-xs text-gray-400">{friend.university}</p>
                  )}
                </div>
              </div>
              <Button onClick={() => handleUnfriend(friend.id)} variant="outline" size="sm">
                Unfriend
              </Button>
            </div>
          ))}
        </div>
      </CardContent>
    </Card>
  )
}
