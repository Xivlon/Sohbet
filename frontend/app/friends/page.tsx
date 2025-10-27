"use client"

import { FriendRequests } from "@/app/components/friend-requests"
import { FriendsList } from "@/app/components/friends-list"

export default function FriendsPage() {
  return (
    <div className="container mx-auto py-8 px-4">
      <h1 className="text-3xl font-bold mb-8">Friends</h1>
      
      <div className="grid gap-6 md:grid-cols-1 lg:grid-cols-2">
        <FriendRequests />
        <FriendsList />
      </div>
    </div>
  )
}
