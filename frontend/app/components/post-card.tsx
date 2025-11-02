"use client"

import { useState } from "react"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent, CardFooter, CardHeader } from "@/app/components/ui/card"
import { Heart, MessageCircle, Share2, MoreVertical } from "lucide-react"
import { apiClient, Post } from "@/app/lib/api-client"
import { usePermission, PERMISSIONS } from "@/app/lib/permissions"
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from "@/app/components/ui/dropdown-menu"
import { CommentThread } from "./comment-thread"

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

  const canDeleteAnyPost = usePermission(PERMISSIONS.DELETE_ANY_POST)
  const canEditAnyPost = usePermission(PERMISSIONS.EDIT_ANY_POST)
  const isOwner = currentUserId === (post.author_id || post.user_id)
  
  // User can edit/delete if they own the post OR have admin permissions
  const canEdit = isOwner || canEditAnyPost
  const canDelete = isOwner || canDeleteAnyPost

  const handleLike = async () => {
    try {
      if (liked) {
        const response = await apiClient.removeReaction(post.id)
        if (response.data || response.status === 200) {
          setLiked(false)
          setLikeCount(Math.max(0, likeCount - 1))
        }
      } else {
        const response = await apiClient.reactToPost(post.id, 'like')
        if (response.data || response.status === 200) {
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
      const response = await apiClient.deletePost(post.id)
      if (response.data || response.status === 200) {
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
          
          {(canEdit || canDelete) && (
            <DropdownMenu>
              <DropdownMenuTrigger asChild>
                <Button variant="ghost" size="sm">
                  <MoreVertical className="h-4 w-4" />
                </Button>
              </DropdownMenuTrigger>
              <DropdownMenuContent align="end">
                {canEdit && (
                  <DropdownMenuItem onClick={() => onEdit?.(post.id)}>
                    Edit
                  </DropdownMenuItem>
                )}
                {canDelete && (
                  <DropdownMenuItem onClick={handleDelete} className="text-red-600">
                    Delete
                  </DropdownMenuItem>
                )}
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
        <CardContent className="pt-4 border-t">
          <CommentThread postId={post.id} currentUserId={currentUserId} />
        </CardContent>
      )}
    </Card>
  )
}
