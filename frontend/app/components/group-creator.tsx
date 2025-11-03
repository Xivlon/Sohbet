"use client"

import { useState } from "react"
import { Button } from "@/app/components/ui/button"
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
import { Textarea } from "@/app/components/ui/textarea"
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/app/components/ui/select"
import { Plus, BookOpen } from "lucide-react"
import { PermissionGate } from "./permission-gate"
import { PERMISSIONS } from "@/app/lib/permissions"
import { apiClient } from '@/app/lib/api-client'
import { useAuth } from '@/app/contexts/auth-context'
import { Separator } from "@/app/components/ui/separator"
import { Badge } from "@/app/components/ui/badge"

interface GroupCreatorProps {
  onGroupCreated?: () => void
}

export function GroupCreator({ onGroupCreated }: GroupCreatorProps) {
  const [open, setOpen] = useState(false)
  const [isCreating, setIsCreating] = useState(false)
  const [formData, setFormData] = useState({
    name: '',
    description: '',
    privacy: 'private',
    category: 'general',
    capacity: '',
    prerequisites: '',
    tags: '',
    meetingSchedule: ''
  })

  const { user } = useAuth()

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    if (!user?.id) return

    setIsCreating(true)
    try {
      const response = await apiClient.createGroup(formData.name, formData.description, formData.privacy)

      if (response.error) {
        alert(response.error || 'Failed to create group')
      } else {
        setFormData({
          name: '',
          description: '',
          privacy: 'private',
          category: 'general',
          capacity: '',
          prerequisites: '',
          tags: '',
          meetingSchedule: ''
        })
        setOpen(false)
        onGroupCreated?.()
      }
    } catch (error) {
      console.error('Error creating group:', error)
      alert('Failed to create group')
    } finally {
      setIsCreating(false)
    }
  }

  return (
    <PermissionGate permission={PERMISSIONS.CREATE_GROUP}>
      <Dialog open={open} onOpenChange={setOpen}>
        <DialogTrigger asChild>
          <Button>
            <Plus className="h-4 w-4 mr-2" />
            Create Group
          </Button>
        </DialogTrigger>
        <DialogContent className="sm:max-w-[700px] max-h-[90vh] overflow-y-auto">
          <form onSubmit={handleSubmit}>
            <DialogHeader>
              <div className="flex items-center gap-2">
                <BookOpen className="h-5 w-5" />
                <DialogTitle>Create New Academic Group</DialogTitle>
              </div>
              <DialogDescription>
                Fill out the application form to create a new academic group for collaboration and discussion.
              </DialogDescription>
            </DialogHeader>
            <div className="grid gap-6 py-4">
              {/* Basic Information Section */}
              <div className="space-y-4">
                <h3 className="text-sm font-semibold flex items-center gap-2">
                  Basic Information
                  <Badge variant="secondary">Required</Badge>
                </h3>
                <div className="grid gap-4">
                  <div className="grid gap-2">
                    <Label htmlFor="name">Group Name *</Label>
                    <Input
                      id="name"
                      value={formData.name}
                      onChange={(e) => setFormData({ ...formData, name: e.target.value })}
                      placeholder="e.g., Computer Science 101 - Fall 2024"
                      required
                    />
                  </div>
                  <div className="grid gap-2">
                    <Label htmlFor="description">Description *</Label>
                    <Textarea
                      id="description"
                      value={formData.description}
                      onChange={(e) => setFormData({ ...formData, description: e.target.value })}
                      placeholder="Describe the purpose, goals, and focus areas of this group..."
                      rows={3}
                      required
                    />
                  </div>
                  <div className="grid grid-cols-2 gap-4">
                    <div className="grid gap-2">
                      <Label htmlFor="category">Category *</Label>
                      <Select
                        value={formData.category}
                        onValueChange={(value) => setFormData({ ...formData, category: value })}
                      >
                        <SelectTrigger id="category">
                          <SelectValue />
                        </SelectTrigger>
                        <SelectContent>
                          <SelectItem value="general">General Course</SelectItem>
                          <SelectItem value="study">Study Group</SelectItem>
                          <SelectItem value="research">Research Group</SelectItem>
                          <SelectItem value="project">Project Team</SelectItem>
                          <SelectItem value="seminar">Seminar/Workshop</SelectItem>
                          <SelectItem value="club">Academic Club</SelectItem>
                        </SelectContent>
                      </Select>
                    </div>
                    <div className="grid gap-2">
                      <Label htmlFor="privacy">Privacy *</Label>
                      <Select
                        value={formData.privacy}
                        onValueChange={(value) => setFormData({ ...formData, privacy: value })}
                      >
                        <SelectTrigger id="privacy">
                          <SelectValue />
                        </SelectTrigger>
                        <SelectContent>
                          <SelectItem value="public">Public - Anyone can join</SelectItem>
                          <SelectItem value="private">Private - Members can invite</SelectItem>
                          <SelectItem value="invite_only">Invite Only - Admin approval</SelectItem>
                        </SelectContent>
                      </Select>
                    </div>
                  </div>
                </div>
              </div>

              <Separator />

              {/* Additional Details Section */}
              <div className="space-y-4">
                <h3 className="text-sm font-semibold flex items-center gap-2">
                  Additional Details
                  <Badge variant="outline">Optional</Badge>
                </h3>
                <div className="grid gap-4">
                  <div className="grid gap-2">
                    <Label htmlFor="capacity">Member Capacity</Label>
                    <Input
                      id="capacity"
                      type="number"
                      value={formData.capacity}
                      onChange={(e) => setFormData({ ...formData, capacity: e.target.value })}
                      placeholder="e.g., 50 (leave empty for unlimited)"
                      min="1"
                    />
                    <p className="text-xs text-muted-foreground">
                      Maximum number of members allowed in this group
                    </p>
                  </div>
                  <div className="grid gap-2">
                    <Label htmlFor="prerequisites">Prerequisites</Label>
                    <Textarea
                      id="prerequisites"
                      value={formData.prerequisites}
                      onChange={(e) => setFormData({ ...formData, prerequisites: e.target.value })}
                      placeholder="List any required prior knowledge or courses..."
                      rows={2}
                    />
                    <p className="text-xs text-muted-foreground">
                      Specify any prerequisites or requirements for joining
                    </p>
                  </div>
                  <div className="grid gap-2">
                    <Label htmlFor="tags">Tags/Topics</Label>
                    <Input
                      id="tags"
                      value={formData.tags}
                      onChange={(e) => setFormData({ ...formData, tags: e.target.value })}
                      placeholder="e.g., Python, Algorithms, Data Science (comma-separated)"
                    />
                    <p className="text-xs text-muted-foreground">
                      Add relevant topics to help others find your group
                    </p>
                  </div>
                  <div className="grid gap-2">
                    <Label htmlFor="meetingSchedule">Meeting Schedule</Label>
                    <Input
                      id="meetingSchedule"
                      value={formData.meetingSchedule}
                      onChange={(e) => setFormData({ ...formData, meetingSchedule: e.target.value })}
                      placeholder="e.g., Tuesdays and Thursdays, 2:00 PM - 4:00 PM"
                    />
                    <p className="text-xs text-muted-foreground">
                      If applicable, specify when the group meets
                    </p>
                  </div>
                </div>
              </div>
            </div>
            <DialogFooter>
              <Button type="button" variant="outline" onClick={() => setOpen(false)}>
                Cancel
              </Button>
              <Button type="submit" disabled={isCreating || !formData.name}>
                {isCreating ? 'Creating...' : 'Create Group'}
              </Button>
            </DialogFooter>
          </form>
        </DialogContent>
      </Dialog>
    </PermissionGate>
  )
}
