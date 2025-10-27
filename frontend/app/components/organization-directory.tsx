"use client"

import { useEffect, useState } from "react"
import { OrganizationCard } from "./organization-card"
import { Skeleton } from "@/app/components/ui/skeleton"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/app/components/ui/tabs"

interface Organization {
  id: number
  name: string
  type: string
  description?: string
  email?: string
  website?: string
  logo_url?: string
  created_at: string
}

interface OrganizationDirectoryProps {
  currentUserId?: number
  onOrganizationSelect?: (orgId: number) => void
}

export function OrganizationDirectory({ currentUserId, onOrganizationSelect }: OrganizationDirectoryProps) {
  const [organizations, setOrganizations] = useState<Organization[]>([])
  const [loading, setLoading] = useState(true)
  const [activeTab, setActiveTab] = useState("all")

  useEffect(() => {
    fetchOrganizations()
  }, [currentUserId])

  const fetchOrganizations = async () => {
    if (!currentUserId) {
      setLoading(false);
      return;
    }

    setLoading(true)
    try {
      const response = await fetch('/api/organizations', {
        headers: {
          'X-User-ID': String(currentUserId),
        },
      })
      if (response.ok) {
        const data = await response.json()
        setOrganizations(data)
      }
    } catch (error) {
      console.error('Error fetching organizations:', error)
    } finally {
      setLoading(false)
    }
  }

  const handleManage = (orgId: number) => {
    window.location.href = `/organizations/${orgId}/manage`
  }

  const handleView = (orgId: number) => {
    if (onOrganizationSelect) {
      onOrganizationSelect(orgId)
    } else {
      window.location.href = `/organizations/${orgId}`
    }
  }

  const filterByType = (type: string) => {
    if (type === 'all') return organizations
    return organizations.filter(org => org.type === type)
  }

  if (loading) {
    return (
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {[1, 2, 3, 4, 5, 6].map((i) => (
          <Skeleton key={i} className="h-64 w-full" />
        ))}
      </div>
    )
  }

  return (
    <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
      <TabsList className="grid w-full max-w-2xl grid-cols-4">
        <TabsTrigger value="all">All</TabsTrigger>
        <TabsTrigger value="club">Clubs</TabsTrigger>
        <TabsTrigger value="department">Departments</TabsTrigger>
        <TabsTrigger value="society">Societies</TabsTrigger>
      </TabsList>
      <TabsContent value="all" className="mt-6">
        {organizations.length === 0 ? (
          <div className="text-center py-12 text-muted-foreground">
            No organizations available yet.
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {organizations.map((org) => (
              <OrganizationCard
                key={org.id}
                organization={org}
                currentUserId={currentUserId}
                onManage={handleManage}
                onView={handleView}
              />
            ))}
          </div>
        )}
      </TabsContent>
      <TabsContent value="club" className="mt-6">
        {filterByType('club').length === 0 ? (
          <div className="text-center py-12 text-muted-foreground">
            No clubs available yet.
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filterByType('club').map((org) => (
              <OrganizationCard
                key={org.id}
                organization={org}
                currentUserId={currentUserId}
                onManage={handleManage}
                onView={handleView}
              />
            ))}
          </div>
        )}
      </TabsContent>
      <TabsContent value="department" className="mt-6">
        {filterByType('department').length === 0 ? (
          <div className="text-center py-12 text-muted-foreground">
            No departments available yet.
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filterByType('department').map((org) => (
              <OrganizationCard
                key={org.id}
                organization={org}
                currentUserId={currentUserId}
                onManage={handleManage}
                onView={handleView}
              />
            ))}
          </div>
        )}
      </TabsContent>
      <TabsContent value="society" className="mt-6">
        {filterByType('society').length === 0 ? (
          <div className="text-center py-12 text-muted-foreground">
            No societies available yet.
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filterByType('society').map((org) => (
              <OrganizationCard
                key={org.id}
                organization={org}
                currentUserId={currentUserId}
                onManage={handleManage}
                onView={handleView}
              />
            ))}
          </div>
        )}
      </TabsContent>
    </Tabs>
  )
}
