"use client"

import React, { useState } from 'react'
import { Plus, X } from 'lucide-react'
import { Button } from './ui/button'
import { Input } from './ui/input'
import { Badge } from './ui/badge'
import { Label } from './ui/label'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from './ui/select'

interface Skill {
  id?: number
  name: string
  proficiency: 'beginner' | 'intermediate' | 'advanced' | 'expert'
}

interface SkillsInterestsEditorProps {
  skills: Skill[]
  interests: string[]
  onSkillsChange: (skills: Skill[]) => void
  onInterestsChange: (interests: string[]) => void
  editable?: boolean
}

export function SkillsInterestsEditor({
  skills,
  interests,
  onSkillsChange,
  onInterestsChange,
  editable = true
}: SkillsInterestsEditorProps) {
  const [newSkill, setNewSkill] = useState('')
  const [newSkillProficiency, setNewSkillProficiency] = useState<Skill['proficiency']>('intermediate')
  const [newInterest, setNewInterest] = useState('')

  const addSkill = () => {
    if (newSkill.trim()) {
      onSkillsChange([...skills, { name: newSkill.trim(), proficiency: newSkillProficiency }])
      setNewSkill('')
      setNewSkillProficiency('intermediate')
    }
  }

  const removeSkill = (index: number) => {
    onSkillsChange(skills.filter((_, i) => i !== index))
  }

  const addInterest = () => {
    if (newInterest.trim() && !interests.includes(newInterest.trim())) {
      onInterestsChange([...interests, newInterest.trim()])
      setNewInterest('')
    }
  }

  const removeInterest = (interest: string) => {
    onInterestsChange(interests.filter(i => i !== interest))
  }

  const getProficiencyColor = (proficiency: Skill['proficiency']) => {
    switch (proficiency) {
      case 'beginner':
        return 'bg-blue-100 text-blue-800 dark:bg-blue-900 dark:text-blue-200'
      case 'intermediate':
        return 'bg-green-100 text-green-800 dark:bg-green-900 dark:text-green-200'
      case 'advanced':
        return 'bg-purple-100 text-purple-800 dark:bg-purple-900 dark:text-purple-200'
      case 'expert':
        return 'bg-orange-100 text-orange-800 dark:bg-orange-900 dark:text-orange-200'
    }
  }

  return (
    <div className="space-y-6">
      {/* Skills Section */}
      <div>
        <Label className="text-base font-semibold mb-3 block">Skills</Label>
        {editable && (
          <div className="flex gap-2 mb-3">
            <Input
              placeholder="Add a skill..."
              value={newSkill}
              onChange={(e) => setNewSkill(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && addSkill()}
              className="flex-1"
            />
            <Select
              value={newSkillProficiency}
              onValueChange={(value) => setNewSkillProficiency(value as Skill['proficiency'])}
            >
              <SelectTrigger className="w-[140px]">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="beginner">Beginner</SelectItem>
                <SelectItem value="intermediate">Intermediate</SelectItem>
                <SelectItem value="advanced">Advanced</SelectItem>
                <SelectItem value="expert">Expert</SelectItem>
              </SelectContent>
            </Select>
            <Button onClick={addSkill} size="icon">
              <Plus className="h-4 w-4" />
            </Button>
          </div>
        )}
        <div className="flex flex-wrap gap-2">
          {skills.length === 0 ? (
            <p className="text-sm text-muted-foreground">No skills added yet</p>
          ) : (
            skills.map((skill, index) => (
              <Badge
                key={index}
                variant="secondary"
                className={`${getProficiencyColor(skill.proficiency)} flex items-center gap-1`}
              >
                {skill.name}
                <span className="text-xs opacity-70">({skill.proficiency})</span>
                {editable && (
                  <button
                    onClick={() => removeSkill(index)}
                    className="ml-1 hover:text-destructive"
                  >
                    <X className="h-3 w-3" />
                  </button>
                )}
              </Badge>
            ))
          )}
        </div>
      </div>

      {/* Interests Section */}
      <div>
        <Label className="text-base font-semibold mb-3 block">Interests</Label>
        {editable && (
          <div className="flex gap-2 mb-3">
            <Input
              placeholder="Add an interest..."
              value={newInterest}
              onChange={(e) => setNewInterest(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && addInterest()}
              className="flex-1"
            />
            <Button onClick={addInterest} size="icon">
              <Plus className="h-4 w-4" />
            </Button>
          </div>
        )}
        <div className="flex flex-wrap gap-2">
          {interests.length === 0 ? (
            <p className="text-sm text-muted-foreground">No interests added yet</p>
          ) : (
            interests.map((interest, index) => (
              <Badge key={index} variant="outline" className="flex items-center gap-1">
                {interest}
                {editable && (
                  <button
                    onClick={() => removeInterest(interest)}
                    className="ml-1 hover:text-destructive"
                  >
                    <X className="h-3 w-3" />
                  </button>
                )}
              </Badge>
            ))
          )}
        </div>
      </div>
    </div>
  )
}
