"use client"

import { useState, useEffect } from "react"
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from "@/app/components/ui/card"
import { Button } from "@/app/components/ui/button"
import { Textarea } from "@/app/components/ui/textarea"
import { Avatar, AvatarFallback, AvatarImage } from "@/app/components/ui/avatar"
import { Badge } from "@/app/components/ui/badge"
import { Separator } from "@/app/components/ui/separator"
import { MessageSquare, Pin, Plus, Send } from "lucide-react"
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/app/components/ui/dialog"
import { Input } from "@/app/components/ui/input"
import { Label } from "@/app/components/ui/label"
import { useAuth } from "@/app/contexts/auth-context"

interface Announcement {
  id: number
  title: string
  content: string
  author: {
    id: number
    name: string
    avatar?: string
    role?: string
  }
  createdAt: string
  isPinned: boolean
  commentsCount: number
}

interface GroupAnnouncementsProps {
  groupId: number
  userRole?: string
}

export function GroupAnnouncements({ groupId, userRole }: GroupAnnouncementsProps) {
  const { user } = useAuth()
  const [announcements, setAnnouncements] = useState<Announcement[]>([])
  const [loading, setLoading] = useState(true)
  const [newAnnouncementOpen, setNewAnnouncementOpen] = useState(false)
  const [newAnnouncement, setNewAnnouncement] = useState({ title: '', content: '' })

  useEffect(() => {
    fetchAnnouncements()
  }, [groupId])

  const fetchAnnouncements = async () => {
    setLoading(true)
    // Mock data for now
    const mockAnnouncements: Announcement[] = [
      {
        id: 1,
        title: "Welcome to the Group!",
        content: "Welcome everyone! This is our main communication hub. Please introduce yourself and let us know your interests in the subject matter. Looking forward to a great semester of collaboration!",
        author: {
          id: 1,
          name: "Dr. Sarah Johnson",
          avatar: undefined,
          role: "Professor"
        },
        createdAt: new Date(Date.now() - 7 * 24 * 60 * 60 * 1000).toISOString(),
        isPinned: true,
        commentsCount: 12
      },
      {
        id: 2,
        title: "Midterm Exam Schedule",
        content: "The midterm exam will be held on October 15th at 10:00 AM in Room 301. The exam will cover chapters 1-5. Please review the study guide posted in the Cloud Access section. Office hours will be extended next week for any questions.",
        author: {
          id: 1,
          name: "Dr. Sarah Johnson",
          avatar: undefined,
          role: "Professor"
        },
        createdAt: new Date(Date.now() - 3 * 24 * 60 * 60 * 1000).toISOString(),
        isPinned: true,
        commentsCount: 8
      },
      {
        id: 3,
        title: "New Study Materials Available",
        content: "I've uploaded additional practice problems and solutions to the Cloud Access section. These should help with your understanding of algorithms and data structures. Feel free to discuss any questions in the comments!",
        author: {
          id: 2,
          name: "Teaching Assistant Mike",
          avatar: undefined,
          role: "TA"
        },
        createdAt: new Date(Date.now() - 1 * 24 * 60 * 60 * 1000).toISOString(),
        isPinned: false,
        commentsCount: 5
      },
      {
        id: 4,
        title: "Group Project Teams",
        content: "Please form teams of 3-4 students for the upcoming group project. Submit your team composition by the end of this week. Project details and requirements are available in the Cloud Access section.",
        author: {
          id: 1,
          name: "Dr. Sarah Johnson",
          avatar: undefined,
          role: "Professor"
        },
        createdAt: new Date(Date.now() - 12 * 60 * 60 * 1000).toISOString(),
        isPinned: false,
        commentsCount: 15
      }
    ]

    setTimeout(() => {
      setAnnouncements(mockAnnouncements)
      setLoading(false)
    }, 500)
  }

  const handleCreateAnnouncement = () => {
    if (!newAnnouncement.title || !newAnnouncement.content) return

    const announcement: Announcement = {
      id: announcements.length + 1,
      title: newAnnouncement.title,
      content: newAnnouncement.content,
      author: {
        id: user?.id || 0,
        name: user?.name || "Unknown User",
        avatar: undefined,
        role: userRole || "member"
      },
      createdAt: new Date().toISOString(),
      isPinned: false,
      commentsCount: 0
    }

    setAnnouncements([announcement, ...announcements])
    setNewAnnouncement({ title: '', content: '' })
    setNewAnnouncementOpen(false)
  }

  const formatDate = (dateString: string) => {
    const date = new Date(dateString)
    const now = new Date()
    const diffInHours = Math.floor((now.getTime() - date.getTime()) / (1000 * 60 * 60))

    if (diffInHours < 24) {
      if (diffInHours < 1) return "Just now"
      return `${diffInHours} hour${diffInHours > 1 ? 's' : ''} ago`
    } else if (diffInHours < 48) {
      return "Yesterday"
    } else {
      return date.toLocaleDateString()
    }
  }

  const canCreateAnnouncement = userRole === 'admin' || userRole === 'moderator'

  return (
    <div className="space-y-6">
      <div className="flex justify-between items-center">
        <h2 className="text-2xl font-bold">Announcements</h2>
        {canCreateAnnouncement && (
          <Dialog open={newAnnouncementOpen} onOpenChange={setNewAnnouncementOpen}>
            <DialogTrigger asChild>
              <Button>
                <Plus className="h-4 w-4 mr-2" />
                New Announcement
              </Button>
            </DialogTrigger>
            <DialogContent className="sm:max-w-[600px]">
              <DialogHeader>
                <DialogTitle>Create Announcement</DialogTitle>
                <DialogDescription>
                  Share important information with all group members.
                </DialogDescription>
              </DialogHeader>
              <div className="grid gap-4 py-4">
                <div className="grid gap-2">
                  <Label htmlFor="title">Title *</Label>
                  <Input
                    id="title"
                    value={newAnnouncement.title}
                    onChange={(e) => setNewAnnouncement({ ...newAnnouncement, title: e.target.value })}
                    placeholder="Enter announcement title"
                  />
                </div>
                <div className="grid gap-2">
                  <Label htmlFor="content">Content *</Label>
                  <Textarea
                    id="content"
                    value={newAnnouncement.content}
                    onChange={(e) => setNewAnnouncement({ ...newAnnouncement, content: e.target.value })}
                    placeholder="Write your announcement here..."
                    rows={6}
                  />
                </div>
              </div>
              <DialogFooter>
                <Button variant="outline" onClick={() => setNewAnnouncementOpen(false)}>
                  Cancel
                </Button>
                <Button
                  onClick={handleCreateAnnouncement}
                  disabled={!newAnnouncement.title || !newAnnouncement.content}
                >
                  <Send className="h-4 w-4 mr-2" />
                  Post Announcement
                </Button>
              </DialogFooter>
            </DialogContent>
          </Dialog>
        )}
      </div>

      {loading ? (
        <Card>
          <CardContent className="p-8 text-center">
            <p className="text-muted-foreground">Loading announcements...</p>
          </CardContent>
        </Card>
      ) : announcements.length === 0 ? (
        <Card>
          <CardContent className="p-8 text-center">
            <p className="text-muted-foreground">No announcements yet.</p>
          </CardContent>
        </Card>
      ) : (
        <div className="space-y-4">
          {announcements
            .sort((a, b) => (b.isPinned ? 1 : 0) - (a.isPinned ? 1 : 0))
            .map((announcement) => (
              <Card key={announcement.id} className={announcement.isPinned ? "border-primary" : ""}>
                <CardHeader>
                  <div className="flex items-start justify-between">
                    <div className="flex items-start gap-3 flex-1">
                      <Avatar>
                        <AvatarImage src={announcement.author.avatar} />
                        <AvatarFallback>
                          {announcement.author.name.split(' ').map(n => n[0]).join('').toUpperCase()}
                        </AvatarFallback>
                      </Avatar>
                      <div className="flex-1">
                        <div className="flex items-center gap-2">
                          <CardTitle className="text-xl">{announcement.title}</CardTitle>
                          {announcement.isPinned && (
                            <Badge variant="secondary" className="flex items-center gap-1">
                              <Pin className="h-3 w-3" />
                              Pinned
                            </Badge>
                          )}
                        </div>
                        <div className="flex items-center gap-2 mt-1">
                          <span className="text-sm font-medium">{announcement.author.name}</span>
                          {announcement.author.role && (
                            <Badge variant="outline" className="text-xs">
                              {announcement.author.role}
                            </Badge>
                          )}
                          <span className="text-sm text-muted-foreground">
                            {formatDate(announcement.createdAt)}
                          </span>
                        </div>
                      </div>
                    </div>
                  </div>
                </CardHeader>
                <CardContent>
                  <p className="text-base leading-relaxed whitespace-pre-wrap">
                    {announcement.content}
                  </p>
                </CardContent>
                <Separator />
                <CardFooter className="pt-4">
                  <Button variant="ghost" size="sm">
                    <MessageSquare className="h-4 w-4 mr-2" />
                    {announcement.commentsCount} Comments
                  </Button>
                </CardFooter>
              </Card>
            ))}
        </div>
      )}
    </div>
  )
}
