"use client"

import { useState, useEffect } from "react"
import { Button } from "@/app/components/ui/button"
import { CommentForm } from "./comment-form"
import { MessageCircle, MoreVertical } from "lucide-react"
import { apiClient, Comment } from "@/app/lib/api-client"
import { usePermission, PERMISSIONS } from "@/app/lib/permissions"
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from "@/app/components/ui/dropdown-menu"

interface CommentThreadProps {
  postId: number
  currentUserId?: number
}

export function CommentThread({ postId, currentUserId }: CommentThreadProps) {
  const [comments, setComments] = useState<Comment[]>([])
  const [loading, setLoading] = useState(true)
  const [replyingTo, setReplyingTo] = useState<number | null>(null)
  const [showCommentForm, setShowCommentForm] = useState(false)

  const canDeleteAnyComment = usePermission(PERMISSIONS.DELETE_ANY_COMMENT)

  useEffect(() => {
    fetchComments()
  }, [postId])

  const fetchComments = async () => {
    try {
      const response = await apiClient.getComments(postId)
      if (response.data) {
        setComments(response.data)
      }
    } catch (error) {
      console.error('Error fetching comments:', error)
    } finally {
      setLoading(false)
    }
  }

  const handleCommentCreated = () => {
    setShowCommentForm(false)
    setReplyingTo(null)
    fetchComments()
  }

  const handleDelete = async (commentId: number) => {
    if (!confirm('Are you sure you want to delete this comment?')) return

    try {
      const response = await apiClient.deleteComment(commentId)
      if (response.data || response.status === 200) {
        // Remove the comment and all its replies
        const removeCommentAndReplies = (comments: Comment[], idToRemove: number): Comment[] => {
          const filtered = comments.filter(c => c.id !== idToRemove)
          // Find and remove all replies to this comment
          const replyIds = comments.filter(c => c.parent_id === idToRemove).map(c => c.id)
          let result = filtered
          for (const replyId of replyIds) {
            result = removeCommentAndReplies(result, replyId)
          }
          return result
        }
        setComments(removeCommentAndReplies(comments, commentId))
      }
    } catch (error) {
      console.error('Error deleting comment:', error)
    }
  }

  const formatDate = (dateString: string) => {
    const date = new Date(dateString)
    const now = new Date()
    const diffMs = now.getTime() - date.getTime()
    const diffMins = Math.floor(diffMs / 60000)
    const diffHours = Math.floor(diffMs / 3600000)
    const diffDays = Math.floor(diffMs / 86400000)

    if (diffMins < 1) return 'just now'
    if (diffMins < 60) return `${diffMins}m ago`
    if (diffHours < 24) return `${diffHours}h ago`
    if (diffDays < 7) return `${diffDays}d ago`
    return date.toLocaleDateString('en-US', { month: 'short', day: 'numeric' })
  }

  const renderComment = (comment: Comment, depth: number = 0) => {
    const isOwner = currentUserId === comment.author_id
    const canDelete = isOwner || canDeleteAnyComment
    const replies = comments.filter(c => c.parent_id === comment.id)

    return (
      <div key={comment.id} className={depth > 0 ? "ml-8 border-l-2 pl-4" : ""}>
        <div className="py-3">
          <div className="flex items-start gap-3">
            <div className="w-8 h-8 rounded-full bg-gray-200 flex items-center justify-center flex-shrink-0">
              {comment.author?.username?.[0]?.toUpperCase() || '?'}
            </div>
            <div className="flex-1 min-w-0">
              <div className="flex items-center gap-2 flex-wrap">
                <span className="font-medium text-sm">
                  {comment.author?.name || comment.author?.username || 'Unknown'}
                </span>
                <span className="text-xs text-gray-500">
                  {formatDate(comment.created_at)}
                </span>
              </div>
              <p className="text-sm mt-1 whitespace-pre-wrap break-words">{comment.content}</p>
              
              <div className="flex items-center gap-4 mt-2">
                <Button
                  variant="ghost"
                  size="sm"
                  className="h-auto p-0 text-xs text-gray-600 hover:text-gray-900"
                  onClick={() => setReplyingTo(replyingTo === comment.id ? null : comment.id)}
                >
                  <MessageCircle className="h-3 w-3 mr-1" />
                  Reply
                </Button>

                {canDelete && (
                  <DropdownMenu>
                    <DropdownMenuTrigger asChild>
                      <Button variant="ghost" size="sm" className="h-auto p-0">
                        <MoreVertical className="h-3 w-3" />
                      </Button>
                    </DropdownMenuTrigger>
                    <DropdownMenuContent align="end">
                      <DropdownMenuItem onClick={() => handleDelete(comment.id)} className="text-red-600">
                        Delete
                      </DropdownMenuItem>
                    </DropdownMenuContent>
                  </DropdownMenu>
                )}
              </div>

              {replyingTo === comment.id && (
                <div className="mt-3">
                  <CommentForm
                    parentCommentId={comment.id}
                    onCommentCreated={handleCommentCreated}
                    onCancel={() => setReplyingTo(null)}
                    placeholder={`Reply to ${comment.author?.username}...`}
                    autoFocus
                  />
                </div>
              )}
            </div>
          </div>
        </div>

        {replies.map(reply => renderComment(reply, depth + 1))}
      </div>
    )
  }

  if (loading) {
    return <div className="text-sm text-gray-500 py-4">Loading comments...</div>
  }

  // Get top-level comments (those without a parent)
  const topLevelComments = comments.filter(c => c.parent_id === null)

  return (
    <div className="space-y-2">
      {topLevelComments.length > 0 && (
        <div className="divide-y">
          {topLevelComments.map(comment => renderComment(comment))}
        </div>
      )}

      {showCommentForm ? (
        <div className="pt-4">
          <CommentForm
            postId={postId}
            onCommentCreated={handleCommentCreated}
            onCancel={() => setShowCommentForm(false)}
            autoFocus
          />
        </div>
      ) : (
        <Button
          variant="ghost"
          size="sm"
          className="w-full"
          onClick={() => setShowCommentForm(true)}
        >
          Add a comment
        </Button>
      )}
    </div>
  )
}
