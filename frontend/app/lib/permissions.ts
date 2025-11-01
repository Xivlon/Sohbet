"use client";

import { useAuth } from '../contexts/auth-context';

// Permission definitions matching backend
export const PERMISSIONS = {
  // Student permissions
  CREATE_POST: 'create_post',
  COMMENT_POST: 'comment_post',
  SEND_FRIEND_REQUEST: 'send_friend_request',
  SEND_MESSAGE: 'send_message',
  JOIN_GROUP: 'join_group',
  
  // Professor permissions
  CREATE_GROUP: 'create_group',
  MANAGE_GROUP: 'manage_group',
  CREATE_PUBLIC_POST: 'create_public_post',
  
  // Admin permissions
  MANAGE_USERS: 'manage_users',
  DELETE_ANY_POST: 'delete_any_post',
  EDIT_ANY_POST: 'edit_any_post',
  DELETE_ANY_COMMENT: 'delete_any_comment',
  EDIT_ANY_COMMENT: 'edit_any_comment',
  DELETE_ANY_FRIENDSHIP: 'delete_any_friendship',
  MANAGE_ORGANIZATIONS: 'manage_organizations',
} as const;

export type Permission = typeof PERMISSIONS[keyof typeof PERMISSIONS];

// Role definitions
export const ROLES = {
  STUDENT: 'Student',
  PROFESSOR: 'Professor',
  ADMIN: 'Admin',
} as const;

export type Role = typeof ROLES[keyof typeof ROLES];

// Role to permissions mapping (matches backend)
const ROLE_PERMISSIONS: Record<Role, Permission[]> = {
  [ROLES.STUDENT]: [
    PERMISSIONS.CREATE_POST,
    PERMISSIONS.COMMENT_POST,
    PERMISSIONS.SEND_FRIEND_REQUEST,
    PERMISSIONS.SEND_MESSAGE,
    PERMISSIONS.JOIN_GROUP,
  ],
  [ROLES.PROFESSOR]: [
    PERMISSIONS.CREATE_POST,
    PERMISSIONS.COMMENT_POST,
    PERMISSIONS.SEND_FRIEND_REQUEST,
    PERMISSIONS.SEND_MESSAGE,
    PERMISSIONS.JOIN_GROUP,
    PERMISSIONS.CREATE_GROUP,
    PERMISSIONS.MANAGE_GROUP,
    PERMISSIONS.CREATE_PUBLIC_POST,
  ],
  [ROLES.ADMIN]: [
    PERMISSIONS.CREATE_POST,
    PERMISSIONS.COMMENT_POST,
    PERMISSIONS.SEND_FRIEND_REQUEST,
    PERMISSIONS.SEND_MESSAGE,
    PERMISSIONS.JOIN_GROUP,
    PERMISSIONS.CREATE_GROUP,
    PERMISSIONS.MANAGE_GROUP,
    PERMISSIONS.CREATE_PUBLIC_POST,
    PERMISSIONS.MANAGE_USERS,
    PERMISSIONS.DELETE_ANY_POST,
    PERMISSIONS.EDIT_ANY_POST,
    PERMISSIONS.DELETE_ANY_COMMENT,
    PERMISSIONS.EDIT_ANY_COMMENT,
    PERMISSIONS.DELETE_ANY_FRIENDSHIP,
    PERMISSIONS.MANAGE_ORGANIZATIONS,
  ],
};

/**
 * Hook to check if the current user has a specific permission
 */
export function usePermission(permission: Permission): boolean {
  const { user } = useAuth();
  
  if (!user) return false;
  
  const userRole = (user.role || ROLES.STUDENT) as Role;
  const rolePermissions = ROLE_PERMISSIONS[userRole] || [];
  
  return rolePermissions.includes(permission);
}

/**
 * Hook to check if the current user has a specific role
 */
export function useRole(role: Role): boolean {
  const { user } = useAuth();
  
  if (!user) return false;
  
  return (user.role || ROLES.STUDENT) === role;
}

/**
 * Hook to get the current user's role
 */
export function useUserRole(): Role {
  const { user } = useAuth();
  
  if (!user) return ROLES.STUDENT;
  
  return (user.role || ROLES.STUDENT) as Role;
}

/**
 * Hook to check if current user is at least the specified role
 * Hierarchy: Student < Professor < Admin
 */
export function useHasRoleOrHigher(role: Role): boolean {
  const currentRole = useUserRole();
  
  const roleHierarchy = {
    [ROLES.STUDENT]: 1,
    [ROLES.PROFESSOR]: 2,
    [ROLES.ADMIN]: 3,
  };
  
  return roleHierarchy[currentRole] >= roleHierarchy[role];
}
