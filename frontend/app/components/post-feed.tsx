"use client"

import { useState, useEffect } from "react"
import { PostCard } from "./post-card"
import { Button } from "@/app/components/ui/button"
import { apiClient, Post } from "@/app/lib/api-client"
import { useAuth } from "@/app/contexts/auth-context"

export function PostFeed() {
  const [posts, setPosts] = useState<Post[]>([])
  const [loading, setLoading] = useState(true)
  const [hasMore, setHasMore] = useState(true)
  const [offset, setOffset] = useState(0)
  const limit = 20
  
  const { user } = useAuth()

  useEffect(() => {
    fetchPosts()
  }, [])

  const fetchPosts = async (reset = false) => {
    try {
      const currentOffset = reset ? 0 : offset
      const response = await apiClient.getPosts(limit, currentOffset)
      
      if (response.data) {
        const newPosts = response.data.posts || []
        if (reset) {
          setPosts(newPosts)
        } else {
          setPosts([...posts, ...newPosts])
        }
        setHasMore(newPosts.length === limit)
        setOffset(currentOffset + newPosts.length)
      }
    } catch (error) {
      console.error('Error fetching posts:', error)
    } finally {
      setLoading(false)
    }
  }

  const loadMore = () => {
    fetchPosts()
  }

  const handleDeletePost = (postId: number) => {
    setPosts(posts.filter(p => p.id !== postId))
  }

  const handleRefresh = () => {
    setOffset(0)
    fetchPosts(true)
  }

  if (loading && posts.length === 0) {
    return (
      <div className="flex justify-center py-8">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-gray-900"></div>
      </div>
    )
  }

  if (posts.length === 0) {
    return (
      <div className="text-center py-8 text-gray-500">
        <p>No posts to display</p>
        <p className="text-sm">Start following friends to see their posts!</p>
      </div>
    )
  }

  return (
    <div className="space-y-4">
      <div className="flex justify-between items-center">
        <h2 className="text-2xl font-bold">Feed</h2>
        <Button onClick={handleRefresh} variant="outline" size="sm">
          Refresh
        </Button>
      </div>

      {posts.map((post) => (
        <PostCard
          key={post.id}
          post={post}
          currentUserId={user?.id}
          onDelete={handleDeletePost}
        />
      ))}

      {hasMore && (
        <div className="flex justify-center py-4">
          <Button onClick={loadMore} variant="outline">
            Load More
          </Button>
        </div>
      )}
    </div>
  )
}
