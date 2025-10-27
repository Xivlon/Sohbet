-- Migration: Add Social Features to Sohbet Academic Platform
-- Date: October 27, 2025
-- Description: Adds role-based permissions, groups, organizations, posts, comments, 
--              friendships, chat, and voice features

-- =============================================================================
-- 1. ROLE-BASED ACCESS CONTROL (RBAC)
-- =============================================================================

-- Roles table: Student, Professor, Admin
CREATE TABLE IF NOT EXISTS roles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,  -- 'Student', 'Professor', 'Admin'
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Role permissions table
CREATE TABLE IF NOT EXISTS role_permissions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    role_id INTEGER NOT NULL,
    permission TEXT NOT NULL,  -- 'create_group', 'manage_users', 'post_public', etc.
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
    UNIQUE(role_id, permission)
);

-- User roles (many-to-many)
CREATE TABLE IF NOT EXISTS user_roles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    role_id INTEGER NOT NULL,
    assigned_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
    UNIQUE(user_id, role_id)
);

-- Add role column to users table (for primary role) if it doesn't exist
-- This is safe to run multiple times as it will only add the column once
-- SQLite doesn't support IF NOT EXISTS for ALTER TABLE, so we'll skip this
-- The users table should already have the role column from user migration

-- =============================================================================
-- 2. GROUPS (Professor-created academic groups)
-- =============================================================================

CREATE TABLE IF NOT EXISTS groups (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    creator_id INTEGER NOT NULL,  -- Must be Professor
    privacy TEXT DEFAULT 'private',  -- 'public', 'private', 'invite_only'
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (creator_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS group_members (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    group_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    role TEXT DEFAULT 'member',  -- 'admin', 'moderator', 'member'
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(group_id, user_id)
);

-- =============================================================================
-- 3. ORGANIZATIONS / CLUBS
-- =============================================================================

CREATE TABLE IF NOT EXISTS organizations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    type TEXT NOT NULL,  -- 'club', 'department', 'society', etc.
    description TEXT,
    email TEXT,
    website TEXT,
    logo_url TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS organization_accounts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    organization_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,  -- Account manager/administrator
    role TEXT NOT NULL,  -- 'owner', 'admin', 'editor'
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (organization_id) REFERENCES organizations(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(organization_id, user_id)
);

-- =============================================================================
-- 4. FRIENDSHIPS / SOCIAL CONNECTIONS
-- =============================================================================

CREATE TABLE IF NOT EXISTS friendships (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    requester_id INTEGER NOT NULL,
    addressee_id INTEGER NOT NULL,
    status TEXT DEFAULT 'pending',  -- 'pending', 'accepted', 'rejected', 'blocked'
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (requester_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (addressee_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(requester_id, addressee_id),
    CHECK (requester_id != addressee_id)
);

CREATE INDEX idx_friendships_status ON friendships(status);
CREATE INDEX idx_friendships_requester ON friendships(requester_id);
CREATE INDEX idx_friendships_addressee ON friendships(addressee_id);

-- =============================================================================
-- 5. POSTS SYSTEM
-- =============================================================================

CREATE TABLE IF NOT EXISTS posts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    author_id INTEGER NOT NULL,
    author_type TEXT DEFAULT 'user',  -- 'user', 'organization'
    content TEXT NOT NULL,
    media_urls TEXT,  -- JSON array of media URLs
    visibility TEXT DEFAULT 'friends',  -- 'public', 'friends', 'private', 'group'
    group_id INTEGER,  -- NULL if not posted to a group
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE
);

CREATE INDEX idx_posts_author ON posts(author_id);
CREATE INDEX idx_posts_created ON posts(created_at DESC);

-- Post reactions (likes, etc.)
CREATE TABLE IF NOT EXISTS post_reactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    reaction_type TEXT DEFAULT 'like',  -- 'like', 'love', 'insightful', etc.
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(post_id, user_id, reaction_type)
);

-- =============================================================================
-- 6. COMMENTS / REPLIES (Threaded)
-- =============================================================================

CREATE TABLE IF NOT EXISTS comments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id INTEGER NOT NULL,
    parent_id INTEGER,  -- NULL for top-level comments, references comment.id for replies
    author_id INTEGER NOT NULL,
    content TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    FOREIGN KEY (parent_id) REFERENCES comments(id) ON DELETE CASCADE,
    FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_comments_post ON comments(post_id);
CREATE INDEX idx_comments_parent ON comments(parent_id);

-- =============================================================================
-- 7. CHAT / MESSAGING
-- =============================================================================

CREATE TABLE IF NOT EXISTS conversations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user1_id INTEGER NOT NULL,
    user2_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_message_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user1_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (user2_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(user1_id, user2_id),
    CHECK (user1_id < user2_id)  -- Ensure consistent ordering
);

CREATE TABLE IF NOT EXISTS messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    conversation_id INTEGER NOT NULL,
    sender_id INTEGER NOT NULL,
    content TEXT NOT NULL,
    media_url TEXT,
    read_at DATETIME,
    delivered_at DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
    FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_messages_conversation ON messages(conversation_id);
CREATE INDEX idx_messages_created ON messages(created_at DESC);

-- =============================================================================
-- 8. MEDIA / FILE UPLOADS
-- =============================================================================

CREATE TABLE IF NOT EXISTS user_media (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    media_type TEXT NOT NULL,  -- 'avatar', 'banner', 'post_image', 'chat_file'
    storage_key TEXT NOT NULL,  -- Object storage key
    file_name TEXT,
    file_size INTEGER,
    mime_type TEXT,
    url TEXT,  -- Signed/public URL
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Add avatar_url and banner_url to users table if they don't exist
-- SQLite doesn't support IF NOT EXISTS for ALTER TABLE
-- These columns should already exist from user migration, so we'll skip them
-- ALTER TABLE users ADD COLUMN avatar_url TEXT;
-- ALTER TABLE users ADD COLUMN banner_url TEXT;

-- =============================================================================
-- 9. VOICE / MURMUR INTEGRATION
-- =============================================================================

CREATE TABLE IF NOT EXISTS voice_channels (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    channel_type TEXT NOT NULL,  -- 'private', 'group', 'public' (Khave)
    group_id INTEGER,
    organization_id INTEGER,
    murmur_channel_id TEXT,  -- External Murmur channel ID
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
    FOREIGN KEY (organization_id) REFERENCES organizations(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS voice_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    channel_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    murmur_session_id TEXT,
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    left_at DATETIME,
    FOREIGN KEY (channel_id) REFERENCES voice_channels(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- =============================================================================
-- 10. SEED DATA - Default Roles
-- =============================================================================

INSERT OR IGNORE INTO roles (name, description) VALUES 
('Student', 'Default student account with basic permissions'),
('Professor', 'Faculty account with group creation and management permissions'),
('Admin', 'Administrator account with full system permissions');

-- Default permissions for each role
INSERT OR IGNORE INTO role_permissions (role_id, permission) VALUES
-- Student permissions
(1, 'create_post'),
(1, 'comment_post'),
(1, 'send_friend_request'),
(1, 'send_message'),
(1, 'join_group'),
-- Professor permissions
(2, 'create_post'),
(2, 'comment_post'),
(2, 'send_friend_request'),
(2, 'send_message'),
(2, 'join_group'),
(2, 'create_group'),
(2, 'manage_group'),
(2, 'create_public_post'),
-- Admin permissions
(3, 'create_post'),
(3, 'comment_post'),
(3, 'send_friend_request'),
(3, 'send_message'),
(3, 'join_group'),
(3, 'create_group'),
(3, 'manage_group'),
(3, 'create_public_post'),
(3, 'manage_users'),
(3, 'delete_any_post'),
(3, 'manage_organizations');

-- Assign default roles to existing users based on their position
UPDATE users SET role = 
    CASE 
        WHEN position = 'Professor' THEN 'Professor'
        WHEN position = 'Student' THEN 'Student'
        ELSE 'Student'
    END
WHERE role IS NULL OR role = 'Student';

-- Create user_roles entries for existing users
INSERT OR IGNORE INTO user_roles (user_id, role_id)
SELECT u.id, r.id
FROM users u
JOIN roles r ON r.name = u.role;

-- =============================================================================
-- MIGRATION COMPLETE
-- =============================================================================
