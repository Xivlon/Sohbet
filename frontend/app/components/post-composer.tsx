"use client"

import { useState } from "react"
import { Button } from "@/app/components/ui/button"
import { Textarea } from "@/app/components/ui/textarea"
import { Card, CardContent, CardHeader, CardTitle } from "@/app/components/ui/card"
import { apiClient } from "@/app/lib/api-client"
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/app/components/ui/select"
import { useTranslations } from 'next-intl'

interface PostComposerProps {
  onPostCreated?: () => void
}

export function PostComposer({ onPostCreated }: PostComposerProps) {
  const [content, setContent] = useState("")
  const [visibility, setVisibility] = useState("public")
  const [posting, setPosting] = useState(false)
  const t = useTranslations('feed')
  const tCommon = useTranslations('common')
  const tPost = useTranslations('post')

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    if (!content.trim()) return

    setPosting(true)
    try {
      const response = await apiClient.createPost(content, visibility)

      if (response.data) {
        setContent("")
        setVisibility("public")
        onPostCreated?.()
      }
    } catch (error) {
      console.error('Error creating post:', error)
    } finally {
      setPosting(false)
    }
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>{t('createPost')}</CardTitle>
      </CardHeader>
      <CardContent>
        <form onSubmit={handleSubmit} className="space-y-4">
          <Textarea
            placeholder={t('whatsOnYourMind')}
            value={content}
            onChange={(e) => setContent(e.target.value)}
            className="min-h-[100px]"
            disabled={posting}
          />

          <div className="flex items-center justify-between">
            <Select value={visibility} onValueChange={setVisibility}>
              <SelectTrigger className="w-[180px]">
                <SelectValue placeholder={t('postVisibility')} />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="public">{t('public')}</SelectItem>
                <SelectItem value="friends">{t('friendsOnly')}</SelectItem>
                <SelectItem value="private">{t('private')}</SelectItem>
              </SelectContent>
            </Select>

            <Button type="submit" disabled={posting || !content.trim()}>
              {posting ? tCommon('processing') + '...' : tPost('share')}
            </Button>
          </div>
        </form>
      </CardContent>
    </Card>
  )
}
