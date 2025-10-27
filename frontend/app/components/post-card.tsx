"use client"

import { useState } from "react"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent, CardFooter, CardHeader } from "@/app/components/ui/card"
import { Heart, MessageCircle, Share2, MoreVertical } from "lucide-react"
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from "@/app/components/ui/dropdown-menu"

interface Post {
  id: number
  author_id: number
  content: string
  visibility: string
  created_at: string
  updated_at?: string
  author?: {
    id: number
    username: string
    name?: string
    avatar_url?: string
  }
}

interface PostCardProps {
  post: Post
  currentUserId?: number
  onDelete?: (postId: number) => void
  onEdit?: (postId: number) => void
}

export function PostCard({ post, currentUserId, onDelete, onEdit }: PostCardProps) {
  const [liked, setLiked] = useState(false)
  const [likeCount, setLikeCount] = useState(0)
  const [showComments, setShowComments] = useState(false)

  const isOwner = currentUserId === post.author_id

  const handleLike = async () => {
    try {
      if (liked) {
        const response = await fetch(`/api/posts/${post.id}/react?reaction_type=like`, {
          method: 'DELETE',
          headers: {
            'X-User-ID': String(currentUserId),
          },
        })
        if (response.ok) {
          setLiked(false)
          setLikeCount(Math.max(0, likeCount - 1))
        }
      } else {
        const response = await fetch(`/api/posts/${post.id}/react`, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            'X-User-ID': String(currentUserId),
          },
          body: JSON.stringify({ reaction_type: 'like' }),
        })
        if (response.ok) {
          setLiked(true)
          setLikeCount(likeCount + 1)
        }
      }
    } catch (error) {
      console.error('Error toggling like:', error)
    }
  }

  const handleDelete = async () => {
    if (!confirm('Are you sure you want to delete this post?')) return

    try {
      const response = await fetch(`/api/posts/${post.id}`, {
        method: 'DELETE',
        headers: {
          'X-User-ID': String(currentUserId),
        },
      })
      if (response.ok) {
        onDelete?.(post.id)
      }
    } catch (error) {
      console.error('Error deleting post:', error)
    }
  }

  const formatDate = (dateString: string) => {
    const date = new Date(dateString)
    return date.toLocaleDateString('en-US', { 
      month: 'short', 
      day: 'numeric',
      year: date.getFullYear() !== new Date().getFullYear() ? 'numeric' : undefined
    })
  }

  return (
    <Card>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-full bg-gray-200 flex items-center justify-center">
              {post.author?.username?.[0]?.toUpperCase() || '?'}
            </div>
            <div>
              <p className="font-medium">{post.author?.name || post.author?.username || 'Unknown'}</p>
              <p className="text-xs text-gray-500">
                {formatDate(post.created_at)} · {post.visibility}
              </p>
            </div>
          </div>
          
          {isOwner && (
            <DropdownMenu>
              <DropdownMenuTrigger asChild>
                <Button variant="ghost" size="sm">
                  <MoreVertical className="h-4 w-4" />
                </Button>
              </DropdownMenuTrigger>
              <DropdownMenuContent align="end">
                <DropdownMenuItem onClick={() => onEdit?.(post.id)}>
                  Edit
                </DropdownMenuItem>
                <DropdownMenuItem onClick={handleDelete} className="text-red-600">
                  Delete
                </DropdownMenuItem>
              </DropdownMenuContent>
            </DropdownMenu>
          )}
        </div>
      </CardHeader>

      <CardContent className="pb-3">
        <p className="whitespace-pre-wrap">{post.content}</p>
      </CardContent>

      <CardFooter className="flex gap-4 pt-3 border-t">
        <Button
          variant="ghost"
          size="sm"
          className={liked ? "text-red-500" : ""}
          onClick={handleLike}
        >
          <Heart className={`h-4 w-4 mr-1 ${liked ? 'fill-current' : ''}`} />
          {likeCount > 0 && likeCount}
        </Button>

        <Button
          variant="ghost"
          size="sm"
          onClick={() => setShowComments(!showComments)}
        >
          <MessageCircle className="h-4 w-4 mr-1" />
          Comment
        </Button>

        <Button variant="ghost" size="sm">
          <Share2 className="h-4 w-4 mr-1" />
          Share
        </Button>
      </CardFooter>

      {showComments && (
        <CardContent className="pt-0 border-t">
          <p className="text-sm text-gray-500 py-4">Comments will appear here</p>
        </CardContent>
      )}
    </Card>
  )
}
