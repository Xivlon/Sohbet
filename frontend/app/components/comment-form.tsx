"use client"

import { useState } from "react"
import { Button } from "@/app/components/ui/button"
import { Textarea } from "@/app/components/ui/textarea"
import { apiClient } from '@/app/lib/api-client'

interface CommentFormProps {
  postId?: number
  parentCommentId?: number
  onCommentCreated?: () => void
  onCancel?: () => void
  placeholder?: string
  autoFocus?: boolean
}

export function CommentForm({
  postId,
  parentCommentId,
  onCommentCreated,
  onCancel,
  placeholder = "Write a comment...",
  autoFocus = false,
}: CommentFormProps) {
  const [content, setContent] = useState("")
  const [posting, setPosting] = useState(false)

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    if (!content.trim()) return

    setPosting(true)
    try {
      let response
      if (parentCommentId) {
        response = await apiClient.replyToComment(parentCommentId, content)
      } else if (postId) {
        response = await apiClient.createComment(postId, content)
      } else {
        // Neither postId nor parentCommentId provided, should not happen
        console.error('CommentForm: No postId or parentCommentId provided')
        setPosting(false)
        return
      }

      if (response && (response.data || response.status === 200)) {
        setContent("")
        onCommentCreated?.()
      }
    } catch (error) {
      console.error('Error creating comment:', error)
    } finally {
      setPosting(false)
    }
  }

  return (
    <form onSubmit={handleSubmit} className="space-y-2">
      <Textarea
        placeholder={placeholder}
        value={content}
        onChange={(e) => setContent(e.target.value)}
        className="min-h-[80px]"
        disabled={posting}
        autoFocus={autoFocus}
      />
      <div className="flex gap-2 justify-end">
        {onCancel && (
          <Button type="button" variant="ghost" onClick={onCancel} disabled={posting}>
            Cancel
          </Button>
        )}
        <Button type="submit" disabled={posting || !content.trim()} size="sm">
          {posting ? "Posting..." : "Comment"}
        </Button>
      </div>
    </form>
  )
}
