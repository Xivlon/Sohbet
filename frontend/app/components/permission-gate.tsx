"use client";

import { ReactNode } from 'react';
import { usePermission, useRole, Permission, Role } from '../lib/permissions';

interface PermissionGateProps {
  children: ReactNode;
  permission?: Permission;
  role?: Role;
  fallback?: ReactNode;
}

/**
 * Component to conditionally render children based on user permissions or role
 */
export function PermissionGate({ children, permission, role, fallback = null }: PermissionGateProps) {
  const hasPermission = usePermission(permission!);
  const hasRole = useRole(role!);
  
  // If both permission and role are specified, user must have both
  if (permission && role) {
    return hasPermission && hasRole ? <>{children}</> : <>{fallback}</>;
  }
  
  // If only permission is specified
  if (permission) {
    return hasPermission ? <>{children}</> : <>{fallback}</>;
  }
  
  // If only role is specified
  if (role) {
    return hasRole ? <>{children}</> : <>{fallback}</>;
  }
  
  // If neither is specified, show children
  return <>{children}</>;
}
