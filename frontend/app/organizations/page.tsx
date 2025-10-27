"use client"

import { OrganizationDirectory } from "@/app/components/organization-directory"
import { useAuth } from "@/app/contexts/auth-context"

export default function OrganizationsPage() {
  const { user } = useAuth()

  return (
    <div className="container mx-auto py-8 px-4">
      <div className="mb-8">
        <h1 className="text-3xl font-bold">Organizations</h1>
        <p className="text-muted-foreground mt-2">
          Explore clubs, departments, and societies on campus
        </p>
      </div>
      <OrganizationDirectory currentUserId={user?.id} />
    </div>
  )
}
