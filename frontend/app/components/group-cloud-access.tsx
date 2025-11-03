"use client"

import { useState, useEffect } from "react"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/app/components/ui/card"
import { Button } from "@/app/components/ui/button"
import { Input } from "@/app/components/ui/input"
import { Label } from "@/app/components/ui/label"
import { Badge } from "@/app/components/ui/badge"
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from "@/app/components/ui/table"
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/app/components/ui/dialog"
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from "@/app/components/ui/dropdown-menu"
import {
  FileText,
  Download,
  MoreVertical,
  Upload,
  Folder,
  FileImage,
  FileCode,
  File as FileIcon,
  Trash2,
  Eye,
  Share2
} from "lucide-react"
import { Textarea } from "@/app/components/ui/textarea"

interface CloudFile {
  id: number
  name: string
  type: 'pdf' | 'image' | 'code' | 'document' | 'folder' | 'other'
  size: string
  uploadedBy: {
    id: number
    name: string
  }
  uploadedAt: string
  description?: string
  downloads: number
}

interface GroupCloudAccessProps {
  groupId: number
  userRole?: string
}

export function GroupCloudAccess({ groupId, userRole }: GroupCloudAccessProps) {
  const [files, setFiles] = useState<CloudFile[]>([])
  const [loading, setLoading] = useState(true)
  const [uploadOpen, setUploadOpen] = useState(false)
  const [newFile, setNewFile] = useState({ name: '', description: '' })
  const [selectedFile, setSelectedFile] = useState<File | null>(null)

  useEffect(() => {
    fetchFiles()
  }, [groupId])

  const fetchFiles = async () => {
    setLoading(true)
    // Mock data for now
    const mockFiles: CloudFile[] = [
      {
        id: 1,
        name: "Lecture Notes - Week 1.pdf",
        type: 'pdf',
        size: "2.4 MB",
        uploadedBy: {
          id: 1,
          name: "Dr. Sarah Johnson"
        },
        uploadedAt: new Date(Date.now() - 7 * 24 * 60 * 60 * 1000).toISOString(),
        description: "Introduction to the course, syllabus overview, and fundamental concepts",
        downloads: 42
      },
      {
        id: 2,
        name: "Assignment 1 - Solutions.pdf",
        type: 'pdf',
        size: "1.8 MB",
        uploadedBy: {
          id: 2,
          name: "Teaching Assistant Mike"
        },
        uploadedAt: new Date(Date.now() - 5 * 24 * 60 * 60 * 1000).toISOString(),
        description: "Detailed solutions for Assignment 1 with explanations",
        downloads: 38
      },
      {
        id: 3,
        name: "Code Examples",
        type: 'folder',
        size: "15.2 MB",
        uploadedBy: {
          id: 2,
          name: "Teaching Assistant Mike"
        },
        uploadedAt: new Date(Date.now() - 4 * 24 * 60 * 60 * 1000).toISOString(),
        description: "Collection of code examples from lectures",
        downloads: 0
      },
      {
        id: 4,
        name: "Study Guide - Midterm.pdf",
        type: 'pdf',
        size: "3.1 MB",
        uploadedBy: {
          id: 1,
          name: "Dr. Sarah Johnson"
        },
        uploadedAt: new Date(Date.now() - 3 * 24 * 60 * 60 * 1000).toISOString(),
        description: "Comprehensive study guide covering chapters 1-5",
        downloads: 56
      },
      {
        id: 5,
        name: "algorithm_visualization.py",
        type: 'code',
        size: "45 KB",
        uploadedBy: {
          id: 3,
          name: "Student Jane Doe"
        },
        uploadedAt: new Date(Date.now() - 2 * 24 * 60 * 60 * 1000).toISOString(),
        description: "Python script for visualizing sorting algorithms",
        downloads: 12
      },
      {
        id: 6,
        name: "Group Project Requirements.docx",
        type: 'document',
        size: "156 KB",
        uploadedBy: {
          id: 1,
          name: "Dr. Sarah Johnson"
        },
        uploadedAt: new Date(Date.now() - 12 * 60 * 60 * 1000).toISOString(),
        description: "Detailed requirements and grading rubric for the final group project",
        downloads: 34
      },
      {
        id: 7,
        name: "Practice Problems - Data Structures.pdf",
        type: 'pdf',
        size: "2.9 MB",
        uploadedBy: {
          id: 2,
          name: "Teaching Assistant Mike"
        },
        uploadedAt: new Date(Date.now() - 1 * 24 * 60 * 60 * 1000).toISOString(),
        description: "Additional practice problems with varying difficulty levels",
        downloads: 28
      }
    ]

    setTimeout(() => {
      setFiles(mockFiles)
      setLoading(false)
    }, 500)
  }

  const getFileIcon = (type: CloudFile['type']) => {
    switch (type) {
      case 'pdf':
      case 'document':
        return <FileText className="h-5 w-5 text-red-500" />
      case 'image':
        return <FileImage className="h-5 w-5 text-blue-500" />
      case 'code':
        return <FileCode className="h-5 w-5 text-green-500" />
      case 'folder':
        return <Folder className="h-5 w-5 text-yellow-500" />
      default:
        return <FileIcon className="h-5 w-5 text-gray-500" />
    }
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

  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0]
    if (file) {
      setSelectedFile(file)
      setNewFile({ ...newFile, name: file.name })
    }
  }

  const handleUpload = () => {
    if (!selectedFile || !newFile.name) return

    const fileType: CloudFile['type'] =
      selectedFile.type.includes('pdf') ? 'pdf' :
      selectedFile.type.includes('image') ? 'image' :
      selectedFile.name.endsWith('.py') || selectedFile.name.endsWith('.js') ? 'code' :
      selectedFile.type.includes('document') ? 'document' :
      'other'

    const cloudFile: CloudFile = {
      id: files.length + 1,
      name: newFile.name,
      type: fileType,
      size: `${(selectedFile.size / 1024 / 1024).toFixed(2)} MB`,
      uploadedBy: {
        id: 0,
        name: "You"
      },
      uploadedAt: new Date().toISOString(),
      description: newFile.description,
      downloads: 0
    }

    setFiles([cloudFile, ...files])
    setNewFile({ name: '', description: '' })
    setSelectedFile(null)
    setUploadOpen(false)
  }

  const canUpload = userRole === 'admin' || userRole === 'moderator' || userRole === 'member'

  return (
    <div className="space-y-6">
      <div className="flex justify-between items-center">
        <div>
          <h2 className="text-2xl font-bold">Cloud Access</h2>
          <p className="text-muted-foreground mt-1">Shared files and resources for the group</p>
        </div>
        {canUpload && (
          <Dialog open={uploadOpen} onOpenChange={setUploadOpen}>
            <DialogTrigger asChild>
              <Button>
                <Upload className="h-4 w-4 mr-2" />
                Upload File
              </Button>
            </DialogTrigger>
            <DialogContent className="sm:max-w-[500px]">
              <DialogHeader>
                <DialogTitle>Upload File</DialogTitle>
                <DialogDescription>
                  Share a file with all group members. Supported formats: PDF, Images, Documents, Code files.
                </DialogDescription>
              </DialogHeader>
              <div className="grid gap-4 py-4">
                <div className="grid gap-2">
                  <Label htmlFor="file">Select File *</Label>
                  <Input
                    id="file"
                    type="file"
                    onChange={handleFileSelect}
                    accept=".pdf,.doc,.docx,.png,.jpg,.jpeg,.gif,.py,.js,.java,.cpp,.c,.txt"
                  />
                </div>
                <div className="grid gap-2">
                  <Label htmlFor="fileName">File Name *</Label>
                  <Input
                    id="fileName"
                    value={newFile.name}
                    onChange={(e) => setNewFile({ ...newFile, name: e.target.value })}
                    placeholder="Enter file name"
                  />
                </div>
                <div className="grid gap-2">
                  <Label htmlFor="description">Description</Label>
                  <Textarea
                    id="description"
                    value={newFile.description}
                    onChange={(e) => setNewFile({ ...newFile, description: e.target.value })}
                    placeholder="Add a brief description (optional)"
                    rows={3}
                  />
                </div>
              </div>
              <DialogFooter>
                <Button variant="outline" onClick={() => setUploadOpen(false)}>
                  Cancel
                </Button>
                <Button
                  onClick={handleUpload}
                  disabled={!selectedFile || !newFile.name}
                >
                  <Upload className="h-4 w-4 mr-2" />
                  Upload
                </Button>
              </DialogFooter>
            </DialogContent>
          </Dialog>
        )}
      </div>

      <Card>
        <CardHeader>
          <CardTitle className="flex items-center justify-between">
            <span>Shared Files ({files.length})</span>
            <Badge variant="outline">{files.reduce((total, file) => {
              const sizeNum = parseFloat(file.size)
              return total + (file.size.includes('MB') ? sizeNum : sizeNum / 1024)
            }, 0).toFixed(1)} MB Total</Badge>
          </CardTitle>
          <CardDescription>
            Access and download course materials, assignments, and shared resources
          </CardDescription>
        </CardHeader>
        <CardContent>
          {loading ? (
            <div className="text-center py-8">
              <p className="text-muted-foreground">Loading files...</p>
            </div>
          ) : files.length === 0 ? (
            <div className="text-center py-8">
              <p className="text-muted-foreground">No files uploaded yet.</p>
            </div>
          ) : (
            <Table>
              <TableHeader>
                <TableRow>
                  <TableHead className="w-[40%]">Name</TableHead>
                  <TableHead>Uploaded By</TableHead>
                  <TableHead>Size</TableHead>
                  <TableHead>Date</TableHead>
                  <TableHead className="text-center">Downloads</TableHead>
                  <TableHead className="text-right">Actions</TableHead>
                </TableRow>
              </TableHeader>
              <TableBody>
                {files.map((file) => (
                  <TableRow key={file.id}>
                    <TableCell>
                      <div className="flex items-center gap-3">
                        {getFileIcon(file.type)}
                        <div>
                          <p className="font-medium">{file.name}</p>
                          {file.description && (
                            <p className="text-sm text-muted-foreground line-clamp-1">
                              {file.description}
                            </p>
                          )}
                        </div>
                      </div>
                    </TableCell>
                    <TableCell className="text-sm">{file.uploadedBy.name}</TableCell>
                    <TableCell className="text-sm">{file.size}</TableCell>
                    <TableCell className="text-sm">{formatDate(file.uploadedAt)}</TableCell>
                    <TableCell className="text-center">
                      <Badge variant="secondary">{file.downloads}</Badge>
                    </TableCell>
                    <TableCell className="text-right">
                      <DropdownMenu>
                        <DropdownMenuTrigger asChild>
                          <Button variant="ghost" size="icon">
                            <MoreVertical className="h-4 w-4" />
                          </Button>
                        </DropdownMenuTrigger>
                        <DropdownMenuContent align="end">
                          <DropdownMenuItem>
                            <Download className="h-4 w-4 mr-2" />
                            Download
                          </DropdownMenuItem>
                          {file.type !== 'folder' && (
                            <DropdownMenuItem>
                              <Eye className="h-4 w-4 mr-2" />
                              Preview
                            </DropdownMenuItem>
                          )}
                          <DropdownMenuItem>
                            <Share2 className="h-4 w-4 mr-2" />
                            Share Link
                          </DropdownMenuItem>
                          {(userRole === 'admin' || userRole === 'moderator') && (
                            <DropdownMenuItem className="text-destructive">
                              <Trash2 className="h-4 w-4 mr-2" />
                              Delete
                            </DropdownMenuItem>
                          )}
                        </DropdownMenuContent>
                      </DropdownMenu>
                    </TableCell>
                  </TableRow>
                ))}
              </TableBody>
            </Table>
          )}
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle>Storage Information</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
            <div className="text-center p-4 bg-muted rounded-lg">
              <p className="text-2xl font-bold">{files.length}</p>
              <p className="text-sm text-muted-foreground">Total Files</p>
            </div>
            <div className="text-center p-4 bg-muted rounded-lg">
              <p className="text-2xl font-bold">
                {files.reduce((total, file) => {
                  const sizeNum = parseFloat(file.size)
                  return total + (file.size.includes('MB') ? sizeNum : sizeNum / 1024)
                }, 0).toFixed(1)} MB
              </p>
              <p className="text-sm text-muted-foreground">Storage Used</p>
            </div>
            <div className="text-center p-4 bg-muted rounded-lg">
              <p className="text-2xl font-bold">
                {files.reduce((total, file) => total + file.downloads, 0)}
              </p>
              <p className="text-sm text-muted-foreground">Total Downloads</p>
            </div>
          </div>
        </CardContent>
      </Card>
    </div>
  )
}
