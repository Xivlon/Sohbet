"use client"

import { useEffect, useState } from "react"
import { OrganizationCard } from "./organization-card"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/app/components/ui/tabs"
import { Skeleton } from "@/app/components/ui/skeleton"
import { apiClient } from "@/app/lib/api-client"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent } from "@/app/components/ui/card"

interface Organization {
  id: number
  name: string
  description?: string
  type: string
  logo_url?: string
  created_at: string
  member_count?: number
}

interface OrganizationDirectoryProps {
  currentUserId?: number
}

export function OrganizationDirectory({ currentUserId }: OrganizationDirectoryProps) {
  const [organizations, setOrganizations] = useState<Organization[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [activeCategory, setActiveCategory] = useState("all")

  useEffect(() => {
    fetchOrganizations()
  }, [])

  const fetchOrganizations = async () => {
    setLoading(true)
    setError(null)
    
    try {
      const response = await apiClient.getOrganizations(100, 0)
      if (response.data) {
        setOrganizations(response.data.organizations || [])
      } else {
        setError(response.error || 'Organizasyonlar yüklenemedi')
      }
    } catch (err) {
      setError('Bir hata oluştu. Lütfen tekrar deneyin.')
      console.error('Error fetching organizations:', err)
    } finally {
      setLoading(false)
    }
  }

  const categories = ["all", "academic", "sports", "arts", "technology", "social"]
  
  const categoryLabels: Record<string, string> = {
    all: "Tümü",
    academic: "Akademik",
    sports: "Spor",
    arts: "Sanat",
    technology: "Teknoloji",
    social: "Sosyal"
  }

  const filteredOrganizations = activeCategory === "all"
    ? organizations
    : organizations.filter((org) => org.type === activeCategory)

  if (loading) {
    return (
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {[1, 2, 3, 4, 5, 6].map((i) => (
          <Skeleton key={i} className="h-48 w-full" />
        ))}
      </div>
    )
  }

  if (error) {
    return (
      <Card>
        <CardContent className="p-8 text-center">
          <p className="text-destructive mb-4">{error}</p>
          <Button onClick={fetchOrganizations}>Tekrar Dene</Button>
        </CardContent>
      </Card>
    )
  }

  return (
    <div className="w-full">
      <Tabs value={activeCategory} onValueChange={setActiveCategory}>
        <TabsList className="grid w-full max-w-3xl grid-cols-3 md:grid-cols-6">
          {categories.map((cat) => (
            <TabsTrigger key={cat} value={cat}>
              {categoryLabels[cat]}
            </TabsTrigger>
          ))}
        </TabsList>

        <div className="mt-6">
          {filteredOrganizations.length === 0 ? (
            <Card>
              <CardContent className="p-8 text-center">
                <p className="text-muted-foreground">
                  {activeCategory === "all" 
                    ? "Henüz organizasyon yok." 
                    : "Bu kategoride organizasyon bulunamadı."}
                </p>
              </CardContent>
            </Card>
          ) : (
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
              {filteredOrganizations.map((org) => (
                <OrganizationCard
                  key={org.id}
                  organization={org}
                  currentUserId={currentUserId}
                />
              ))}
            </div>
          )}
        </div>
      </Tabs>
    </div>
  )
}
