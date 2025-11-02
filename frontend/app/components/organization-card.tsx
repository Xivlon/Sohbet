"use client"

import { Button } from "@/app/components/ui/button"
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from "@/app/components/ui/card"
import { Badge } from "@/app/components/ui/badge"
import { Building2, Globe, Mail, Settings } from "lucide-react"
import { Organization } from "@/app/lib/api-client"

interface OrganizationCardProps {
  organization: Organization
  currentUserId?: number
  canManage?: boolean
  onManage?: (orgId: number) => void
  onView?: (orgId: number) => void
}

export function OrganizationCard({ organization, canManage, onManage, onView }: OrganizationCardProps) {
  const getTypeColor = (type: string) => {
    switch (type) {
      case 'club':
        return 'bg-blue-100 text-blue-800'
      case 'department':
        return 'bg-green-100 text-green-800'
      case 'society':
        return 'bg-purple-100 text-purple-800'
      default:
        return 'bg-gray-100 text-gray-800'
    }
  }

  const getTypeLabel = (type: string) => {
    return type.charAt(0).toUpperCase() + type.slice(1)
  }

  return (
    <Card className="hover:shadow-lg transition-shadow">
      <CardHeader>
        <div className="flex items-start justify-between">
          <div className="flex items-start gap-3 flex-1">
            {organization.logo_url ? (
              <img
                src={organization.logo_url}
                alt={organization.name}
                className="h-12 w-12 rounded-lg object-cover"
              />
            ) : (
              <div className="h-12 w-12 rounded-lg bg-muted flex items-center justify-center">
                <Building2 className="h-6 w-6 text-muted-foreground" />
              </div>
            )}
            <div className="flex-1">
              <CardTitle className="text-xl">{organization.name}</CardTitle>
              <Badge className={`mt-2 ${getTypeColor(organization.type || organization.category || 'other')}`}>
                {getTypeLabel(organization.type || organization.category || 'Diğer')}
              </Badge>
            </div>
          </div>
          {canManage && (
            <Button
              variant="ghost"
              size="icon"
              onClick={() => onManage?.(organization.id)}
            >
              <Settings className="h-4 w-4" />
            </Button>
          )}
        </div>
        {organization.description && (
          <CardDescription className="mt-3">{organization.description}</CardDescription>
        )}
      </CardHeader>
      <CardContent>
        <div className="space-y-2">
          {organization.email && (
            <div className="flex items-center gap-2 text-sm text-muted-foreground">
              <Mail className="h-4 w-4" />
              <a href={`mailto:${organization.email}`} className="hover:underline">
                {organization.email}
              </a>
            </div>
          )}
          {organization.website && (
            <div className="flex items-center gap-2 text-sm text-muted-foreground">
              <Globe className="h-4 w-4" />
              <a href={organization.website} target="_blank" rel="noopener noreferrer" className="hover:underline">
                {organization.website}
              </a>
            </div>
          )}
        </div>
      </CardContent>
      <CardFooter>
        <Button
          variant="outline"
          onClick={() => onView?.(organization.id)}
          className="w-full"
        >
          View Organization
        </Button>
      </CardFooter>
    </Card>
  )
}
