"use client"

import React, { useState, useEffect } from 'react'
import { Bookmark } from 'lucide-react'
import { Button } from './ui/button'
import { cn } from '../lib/utils'
import { apiClient } from '../lib/api-client'
import { useAuth } from '../contexts/auth-context'

interface BookmarkButtonProps {
  postId: number
  className?: string
  showLabel?: boolean
}

export function BookmarkButton({ postId, className, showLabel = false }: BookmarkButtonProps) {
  const [isBookmarked, setIsBookmarked] = useState(false)
  const [isLoading, setIsLoading] = useState(false)
  const { user } = useAuth()

  useEffect(() => {
    if (user) {
      checkBookmarkStatus()
    }
  }, [postId, user])

  const checkBookmarkStatus = async () => {
    try {
      const response = await apiClient.get(`/api/posts/${postId}/bookmark`)
      if (response.ok) {
        const data = await response.json()
        setIsBookmarked(data.is_bookmarked || false)
      }
    } catch (error) {
      console.error('Failed to check bookmark status:', error)
    }
  }

  const toggleBookmark = async () => {
    if (!user || isLoading) return

    setIsLoading(true)
    try {
      if (isBookmarked) {
        const response = await apiClient.delete(`/api/posts/${postId}/bookmark`)
        if (response.ok) {
          setIsBookmarked(false)
        }
      } else {
        const response = await apiClient.post(`/api/posts/${postId}/bookmark`)
        if (response.ok) {
          setIsBookmarked(true)
        }
      }
    } catch (error) {
      console.error('Failed to toggle bookmark:', error)
    } finally {
      setIsLoading(false)
    }
  }

  if (!user) return null

  return (
    <Button
      variant="ghost"
      size={showLabel ? 'sm' : 'icon'}
      onClick={toggleBookmark}
      disabled={isLoading}
      className={cn(className)}
    >
      <Bookmark
        className={cn(
          "h-4 w-4",
          isBookmarked && "fill-current text-primary",
          showLabel && "mr-2"
        )}
      />
      {showLabel && (isBookmarked ? 'Saved' : 'Save')}
    </Button>
  )
}
