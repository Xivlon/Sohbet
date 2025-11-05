"use client"

import React, { useState, useEffect, useCallback } from 'react'
import { Search, X, Filter, Users, FileText, Users2 } from 'lucide-react'
import { Input } from './ui/input'
import { Button } from './ui/button'
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from './ui/dialog'
import { Tabs, TabsContent, TabsList, TabsTrigger } from './ui/tabs'
import { ScrollArea } from './ui/scroll-area'
import { Avatar, AvatarFallback, AvatarImage } from './ui/avatar'
import { Badge } from './ui/badge'
import { apiClient } from '../lib/api-client'
import { useRouter } from 'next/navigation'
import debounce from 'lodash/debounce'

interface SearchResult {
  users: User[]
  posts: Post[]
  groups: Group[]
}

interface User {
  id: number
  username: string
  full_name: string
  avatar_url?: string
  university?: string
  department?: string
}

interface Post {
  id: number
  content: string
  author_id: number
  author_username: string
  created_at: string
}

interface Group {
  id: number
  name: string
  description: string
  member_count?: number
}

export function AdvancedSearch() {
  const [isOpen, setIsOpen] = useState(false)
  const [query, setQuery] = useState('')
  const [results, setResults] = useState<SearchResult>({
    users: [],
    posts: [],
    groups: []
  })
  const [isSearching, setIsSearching] = useState(false)
  const [activeTab, setActiveTab] = useState('all')
  const router = useRouter()

  const performSearch = async (searchQuery: string) => {
    if (!searchQuery.trim()) {
      setResults({ users: [], posts: [], groups: [] })
      return
    }

    setIsSearching(true)
    try {
      const response = await apiClient.get(`/api/search?q=${encodeURIComponent(searchQuery)}`)
      if (response.ok) {
        const data = await response.json()
        setResults({
          users: data.users || [],
          posts: data.posts || [],
          groups: data.groups || []
        })
      }
    } catch (error) {
      console.error('Search failed:', error)
    } finally {
      setIsSearching(false)
    }
  }

  const debouncedSearch = useCallback(
    debounce((searchQuery: string) => performSearch(searchQuery), 300),
    []
  )

  useEffect(() => {
    debouncedSearch(query)
  }, [query, debouncedSearch])

  const handleUserClick = (userId: number) => {
    router.push(`/profile/${userId}`)
    setIsOpen(false)
  }

  const handlePostClick = (postId: number) => {
    router.push(`/posts/${postId}`)
    setIsOpen(false)
  }

  const handleGroupClick = (groupId: number) => {
    router.push(`/groups/${groupId}`)
    setIsOpen(false)
  }

  const totalResults = results.users.length + results.posts.length + results.groups.length

  return (
    <Dialog open={isOpen} onOpenChange={setIsOpen}>
      <DialogTrigger asChild>
        <Button variant="outline" className="w-full md:w-64">
          <Search className="h-4 w-4 mr-2" />
          Search...
        </Button>
      </DialogTrigger>
      <DialogContent className="max-w-2xl max-h-[80vh]">
        <DialogHeader>
          <DialogTitle>Advanced Search</DialogTitle>
        </DialogHeader>
        <div className="space-y-4">
          <div className="relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-muted-foreground" />
            <Input
              placeholder="Search users, posts, groups..."
              value={query}
              onChange={(e) => setQuery(e.target.value)}
              className="pl-10 pr-10"
              autoFocus
            />
            {query && (
              <Button
                variant="ghost"
                size="icon"
                className="absolute right-1 top-1/2 transform -translate-y-1/2 h-7 w-7"
                onClick={() => setQuery('')}
              >
                <X className="h-4 w-4" />
              </Button>
            )}
          </div>

          {isSearching && (
            <div className="text-center py-8 text-muted-foreground">
              Searching...
            </div>
          )}

          {!isSearching && query && totalResults === 0 && (
            <div className="text-center py-8 text-muted-foreground">
              <Search className="h-12 w-12 mx-auto mb-2 opacity-50" />
              <p>No results found for &quot;{query}&quot;</p>
            </div>
          )}

          {!isSearching && totalResults > 0 && (
            <Tabs value={activeTab} onValueChange={setActiveTab}>
              <TabsList className="grid w-full grid-cols-4">
                <TabsTrigger value="all">
                  All ({totalResults})
                </TabsTrigger>
                <TabsTrigger value="users">
                  <Users className="h-4 w-4 mr-1" />
                  Users ({results.users.length})
                </TabsTrigger>
                <TabsTrigger value="posts">
                  <FileText className="h-4 w-4 mr-1" />
                  Posts ({results.posts.length})
                </TabsTrigger>
                <TabsTrigger value="groups">
                  <Users2 className="h-4 w-4 mr-1" />
                  Groups ({results.groups.length})
                </TabsTrigger>
              </TabsList>

              <ScrollArea className="h-[400px] mt-4">
                <TabsContent value="all" className="space-y-4">
                  {results.users.length > 0 && (
                    <div>
                      <h4 className="font-semibold mb-2 flex items-center">
                        <Users className="h-4 w-4 mr-2" />
                        Users
                      </h4>
                      <div className="space-y-2">
                        {results.users.map((user) => (
                          <UserResult key={user.id} user={user} onClick={handleUserClick} />
                        ))}
                      </div>
                    </div>
                  )}

                  {results.posts.length > 0 && (
                    <div>
                      <h4 className="font-semibold mb-2 flex items-center">
                        <FileText className="h-4 w-4 mr-2" />
                        Posts
                      </h4>
                      <div className="space-y-2">
                        {results.posts.map((post) => (
                          <PostResult key={post.id} post={post} onClick={handlePostClick} />
                        ))}
                      </div>
                    </div>
                  )}

                  {results.groups.length > 0 && (
                    <div>
                      <h4 className="font-semibold mb-2 flex items-center">
                        <Users2 className="h-4 w-4 mr-2" />
                        Groups
                      </h4>
                      <div className="space-y-2">
                        {results.groups.map((group) => (
                          <GroupResult key={group.id} group={group} onClick={handleGroupClick} />
                        ))}
                      </div>
                    </div>
                  )}
                </TabsContent>

                <TabsContent value="users" className="space-y-2">
                  {results.users.map((user) => (
                    <UserResult key={user.id} user={user} onClick={handleUserClick} />
                  ))}
                </TabsContent>

                <TabsContent value="posts" className="space-y-2">
                  {results.posts.map((post) => (
                    <PostResult key={post.id} post={post} onClick={handlePostClick} />
                  ))}
                </TabsContent>

                <TabsContent value="groups" className="space-y-2">
                  {results.groups.map((group) => (
                    <GroupResult key={group.id} group={group} onClick={handleGroupClick} />
                  ))}
                </TabsContent>
              </ScrollArea>
            </Tabs>
          )}
        </div>
      </DialogContent>
    </Dialog>
  )
}

function UserResult({ user, onClick }: { user: User; onClick: (id: number) => void }) {
  return (
    <div
      className="flex items-center gap-3 p-3 rounded-lg hover:bg-accent cursor-pointer transition-colors"
      onClick={() => onClick(user.id)}
    >
      <Avatar>
        <AvatarImage src={user.avatar_url} />
        <AvatarFallback>{user.username[0].toUpperCase()}</AvatarFallback>
      </Avatar>
      <div className="flex-1 min-w-0">
        <p className="font-medium">{user.full_name || user.username}</p>
        <p className="text-sm text-muted-foreground">@{user.username}</p>
        {user.university && (
          <p className="text-xs text-muted-foreground">{user.university} • {user.department}</p>
        )}
      </div>
    </div>
  )
}

function PostResult({ post, onClick }: { post: Post; onClick: (id: number) => void }) {
  return (
    <div
      className="p-3 rounded-lg hover:bg-accent cursor-pointer transition-colors"
      onClick={() => onClick(post.id)}
    >
      <p className="text-sm line-clamp-2">{post.content}</p>
      <div className="flex items-center gap-2 mt-2 text-xs text-muted-foreground">
        <span>by @{post.author_username}</span>
        <span>•</span>
        <span>{new Date(post.created_at).toLocaleDateString()}</span>
      </div>
    </div>
  )
}

function GroupResult({ group, onClick }: { group: Group; onClick: (id: number) => void }) {
  return (
    <div
      className="p-3 rounded-lg hover:bg-accent cursor-pointer transition-colors"
      onClick={() => onClick(group.id)}
    >
      <div className="flex items-start justify-between">
        <div className="flex-1">
          <p className="font-medium">{group.name}</p>
          <p className="text-sm text-muted-foreground line-clamp-2">{group.description}</p>
        </div>
        {group.member_count !== undefined && (
          <Badge variant="secondary" className="ml-2">
            {group.member_count} members
          </Badge>
        )}
      </div>
    </div>
  )
}
