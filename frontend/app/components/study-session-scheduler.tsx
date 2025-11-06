"use client"

import React, { useState, useEffect } from 'react'
import { Calendar, Clock, MapPin, Users, Video, Plus } from 'lucide-react'
import { Button } from './ui/button'
import { Input } from './ui/input'
import { Label } from './ui/label'
import { Textarea } from './ui/textarea'
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
  DialogFooter,
} from './ui/dialog'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from './ui/select'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from './ui/card'
import { Badge } from './ui/badge'
import { api } from '../lib/api-helpers'
import { useAuth } from '../contexts/auth-context'

interface StudySession {
  id: number
  title: string
  description: string
  location: string
  start_time: string
  end_time: string
  created_by: number
  max_participants?: number
  participant_count?: number
  is_participating?: boolean
  voice_channel_id?: number
}

interface StudySessionSchedulerProps {
  groupId: number
  onSessionCreated?: (session: StudySession) => void
}

export function StudySessionScheduler({ groupId, onSessionCreated }: StudySessionSchedulerProps) {
  const [sessions, setSessions] = useState<StudySession[]>([])
  const [isCreateOpen, setIsCreateOpen] = useState(false)
  const [isLoading, setIsLoading] = useState(false)
  const { user } = useAuth()

  const [formData, setFormData] = useState({
    title: '',
    description: '',
    location: 'online',
    startDate: '',
    startTime: '',
    endDate: '',
    endTime: '',
    maxParticipants: ''
  })

  useEffect(() => {
    fetchSessions()
  }, [groupId])

  const fetchSessions = async () => {
    try {
      const response = await api.get(`/api/groups/${groupId}/sessions`)
      if (response.ok) {
        const data = await response.json()
        setSessions(data.sessions || [])
      }
    } catch (error) {
      console.error('Failed to fetch sessions:', error)
    }
  }

  const createSession = async () => {
    if (!formData.title || !formData.startDate || !formData.startTime) {
      return
    }

    setIsLoading(true)
    try {
      const startDateTime = new Date(`${formData.startDate}T${formData.startTime}`)
      const endDateTime = formData.endDate && formData.endTime
        ? new Date(`${formData.endDate}T${formData.endTime}`)
        : new Date(startDateTime.getTime() + 60 * 60 * 1000) // Default 1 hour

      const response = await api.post(`/api/groups/${groupId}/sessions`, {
        title: formData.title,
        description: formData.description,
        location: formData.location,
        start_time: startDateTime.toISOString(),
        end_time: endDateTime.toISOString(),
        max_participants: formData.maxParticipants ? parseInt(formData.maxParticipants) : null
      })

      if (response.ok) {
        const newSession = await response.json()
        setSessions([newSession, ...sessions])
        setIsCreateOpen(false)
        setFormData({
          title: '',
          description: '',
          location: 'online',
          startDate: '',
          startTime: '',
          endDate: '',
          endTime: '',
          maxParticipants: ''
        })
        onSessionCreated?.(newSession)
      }
    } catch (error) {
      console.error('Failed to create session:', error)
    } finally {
      setIsLoading(false)
    }
  }

  const joinSession = async (sessionId: number) => {
    try {
      const response = await api.post(`/api/sessions/${sessionId}/join`)
      if (response.ok) {
        fetchSessions()
      }
    } catch (error) {
      console.error('Failed to join session:', error)
    }
  }

  const leaveSession = async (sessionId: number) => {
    try {
      const response = await api.post(`/api/sessions/${sessionId}/leave`)
      if (response.ok) {
        fetchSessions()
      }
    } catch (error) {
      console.error('Failed to leave session:', error)
    }
  }

  const formatDateTime = (dateTimeStr: string) => {
    const date = new Date(dateTimeStr)
    return {
      date: date.toLocaleDateString(),
      time: date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
    }
  }

  const isUpcoming = (startTime: string) => {
    return new Date(startTime) > new Date()
  }

  const upcomingSessions = sessions.filter(s => isUpcoming(s.start_time))
  const pastSessions = sessions.filter(s => !isUpcoming(s.start_time))

  return (
    <div className="space-y-4">
      <div className="flex items-center justify-between">
        <h3 className="text-lg font-semibold">Study Sessions</h3>
        <Dialog open={isCreateOpen} onOpenChange={setIsCreateOpen}>
          <DialogTrigger asChild>
            <Button>
              <Plus className="h-4 w-4 mr-2" />
              Schedule Session
            </Button>
          </DialogTrigger>
          <DialogContent className="max-w-md">
            <DialogHeader>
              <DialogTitle>Schedule Study Session</DialogTitle>
            </DialogHeader>
            <div className="space-y-4">
              <div>
                <Label htmlFor="title">Title *</Label>
                <Input
                  id="title"
                  value={formData.title}
                  onChange={(e) => setFormData({ ...formData, title: e.target.value })}
                  placeholder="Weekly study group"
                />
              </div>

              <div>
                <Label htmlFor="description">Description</Label>
                <Textarea
                  id="description"
                  value={formData.description}
                  onChange={(e) => setFormData({ ...formData, description: e.target.value })}
                  placeholder="Reviewing chapter 5..."
                  rows={3}
                />
              </div>

              <div>
                <Label htmlFor="location">Location *</Label>
                <Select
                  value={formData.location}
                  onValueChange={(value) => setFormData({ ...formData, location: value })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="online">Online (Voice Chat)</SelectItem>
                    <SelectItem value="library">Library</SelectItem>
                    <SelectItem value="classroom">Classroom</SelectItem>
                    <SelectItem value="cafeteria">Cafeteria</SelectItem>
                    <SelectItem value="other">Other</SelectItem>
                  </SelectContent>
                </Select>
              </div>

              <div className="grid grid-cols-2 gap-4">
                <div>
                  <Label htmlFor="startDate">Start Date *</Label>
                  <Input
                    id="startDate"
                    type="date"
                    value={formData.startDate}
                    onChange={(e) => setFormData({ ...formData, startDate: e.target.value })}
                  />
                </div>
                <div>
                  <Label htmlFor="startTime">Start Time *</Label>
                  <Input
                    id="startTime"
                    type="time"
                    value={formData.startTime}
                    onChange={(e) => setFormData({ ...formData, startTime: e.target.value })}
                  />
                </div>
              </div>

              <div className="grid grid-cols-2 gap-4">
                <div>
                  <Label htmlFor="endDate">End Date</Label>
                  <Input
                    id="endDate"
                    type="date"
                    value={formData.endDate}
                    onChange={(e) => setFormData({ ...formData, endDate: e.target.value })}
                  />
                </div>
                <div>
                  <Label htmlFor="endTime">End Time</Label>
                  <Input
                    id="endTime"
                    type="time"
                    value={formData.endTime}
                    onChange={(e) => setFormData({ ...formData, endTime: e.target.value })}
                  />
                </div>
              </div>

              <div>
                <Label htmlFor="maxParticipants">Max Participants (optional)</Label>
                <Input
                  id="maxParticipants"
                  type="number"
                  min="1"
                  value={formData.maxParticipants}
                  onChange={(e) => setFormData({ ...formData, maxParticipants: e.target.value })}
                  placeholder="Unlimited"
                />
              </div>
            </div>
            <DialogFooter>
              <Button variant="outline" onClick={() => setIsCreateOpen(false)}>
                Cancel
              </Button>
              <Button onClick={createSession} disabled={isLoading}>
                {isLoading ? 'Creating...' : 'Create Session'}
              </Button>
            </DialogFooter>
          </DialogContent>
        </Dialog>
      </div>

      {upcomingSessions.length === 0 && pastSessions.length === 0 ? (
        <Card>
          <CardContent className="py-8 text-center text-muted-foreground">
            <Calendar className="h-12 w-12 mx-auto mb-2 opacity-50" />
            <p>No study sessions scheduled yet</p>
            <p className="text-sm mt-1">Create the first session to get started!</p>
          </CardContent>
        </Card>
      ) : (
        <>
          {upcomingSessions.length > 0 && (
            <div className="space-y-3">
              <h4 className="text-sm font-medium text-muted-foreground">Upcoming Sessions</h4>
              {upcomingSessions.map((session) => (
                <SessionCard
                  key={session.id}
                  session={session}
                  onJoin={joinSession}
                  onLeave={leaveSession}
                />
              ))}
            </div>
          )}

          {pastSessions.length > 0 && (
            <div className="space-y-3">
              <h4 className="text-sm font-medium text-muted-foreground">Past Sessions</h4>
              {pastSessions.map((session) => (
                <SessionCard
                  key={session.id}
                  session={session}
                  isPast
                />
              ))}
            </div>
          )}
        </>
      )}
    </div>
  )
}

function SessionCard({
  session,
  onJoin,
  onLeave,
  isPast = false
}: {
  session: StudySession
  onJoin?: (id: number) => void
  onLeave?: (id: number) => void
  isPast?: boolean
}) {
  const start = formatDateTime(session.start_time)
  const end = formatDateTime(session.end_time)

  return (
    <Card className={isPast ? 'opacity-60' : ''}>
      <CardHeader>
        <div className="flex items-start justify-between">
          <div className="flex-1">
            <CardTitle className="text-base">{session.title}</CardTitle>
            <CardDescription>{session.description}</CardDescription>
          </div>
          {!isPast && !session.is_participating && onJoin && (
            <Button size="sm" onClick={() => onJoin(session.id)}>
              Join
            </Button>
          )}
          {!isPast && session.is_participating && onLeave && (
            <Button size="sm" variant="outline" onClick={() => onLeave(session.id)}>
              Leave
            </Button>
          )}
        </div>
      </CardHeader>
      <CardContent className="space-y-2">
        <div className="flex items-center gap-2 text-sm">
          <Calendar className="h-4 w-4 text-muted-foreground" />
          <span>{start.date}</span>
        </div>
        <div className="flex items-center gap-2 text-sm">
          <Clock className="h-4 w-4 text-muted-foreground" />
          <span>{start.time} - {end.time}</span>
        </div>
        <div className="flex items-center gap-2 text-sm">
          {session.location === 'online' ? (
            <Video className="h-4 w-4 text-muted-foreground" />
          ) : (
            <MapPin className="h-4 w-4 text-muted-foreground" />
          )}
          <span className="capitalize">{session.location}</span>
        </div>
        <div className="flex items-center gap-2">
          <Users className="h-4 w-4 text-muted-foreground" />
          <span className="text-sm">
            {session.participant_count || 0}
            {session.max_participants && ` / ${session.max_participants}`} participants
          </span>
        </div>
      </CardContent>
    </Card>
  )
}

function formatDateTime(dateTimeStr: string) {
  const date = new Date(dateTimeStr)
  return {
    date: date.toLocaleDateString(),
    time: date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
  }
}
