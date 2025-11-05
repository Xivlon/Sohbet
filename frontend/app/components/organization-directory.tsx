"use client"

import { useEffect, useState } from "react"
import { useTranslations } from "next-intl"
import { OrganizationCard } from "./organization-card"
import { Tabs, TabsList, TabsTrigger } from "@/app/components/ui/tabs"
import { Skeleton } from "@/app/components/ui/skeleton"
import { apiClient, Organization } from "@/app/lib/api-client"
import { Button } from "@/app/components/ui/button"
import { Card, CardContent } from "@/app/components/ui/card"

interface OrganizationDirectoryProps {
  currentUserId?: number
}

export function OrganizationDirectory({ currentUserId }: OrganizationDirectoryProps) {
  const t = useTranslations('organizations')
  const tSearch = useTranslations('search')
  const tErrors = useTranslations('errors')

  const [organizations, setOrganizations] = useState<Organization[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [activeCategory, setActiveCategory] = useState("all")

  useEffect(() => {
    // Only fetch organizations if user is authenticated
    if (currentUserId) {
      fetchOrganizations()
    }
  }, [currentUserId])

  const fetchOrganizations = async () => {
    setLoading(true)
    setError(null)

    try {
      const response = await apiClient.getOrganizations(100, 0)
      if (response.data) {
        setOrganizations(response.data.organizations || [])
      } else {
        setError(response.error || tErrors('somethingWentWrong'))
      }
    } catch (err) {
      setError(`${tErrors('somethingWentWrong')}. ${tErrors('tryAgain')}.`)
      console.error('Error fetching organizations:', err)
    } finally {
      setLoading(false)
    }
  }

  const categories = ["all", "academic", "sports", "arts", "technology", "social"]

  const getCategoryLabel = (category: string): string => {
    switch (category) {
      case "all":
        return tSearch('all')
      case "academic":
        return "Akademik"
      case "sports":
        return "Spor"
      case "arts":
        return "Sanat"
      case "technology":
        return "Teknoloji"
      case "social":
        return "Sosyal"
      default:
        return category
    }
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
          <Button onClick={fetchOrganizations}>{tErrors('tryAgain')}</Button>
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
              {getCategoryLabel(cat)}
            </TabsTrigger>
          ))}
        </TabsList>

        <div className="mt-6">
          {filteredOrganizations.length === 0 ? (
            <Card>
              <CardContent className="p-8 text-center">
                <p className="text-muted-foreground">
                  {tSearch('noResults')}
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
