-- Migration: Enhanced Features for Sohbet Academic Platform
-- Date: November 5, 2025
-- Description: Adds notifications, user presence, bookmarks, skills/interests,
--              hashtags, mentions, study sessions, and analytics

-- =============================================================================
-- 1. NOTIFICATIONS SYSTEM
-- =============================================================================

CREATE TABLE IF NOT EXISTS notifications (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    type TEXT NOT NULL,  -- 'friend_request', 'friend_accepted', 'post_like', 'post_comment',
                         -- 'mention', 'group_invite', 'study_session', 'announcement'
    title TEXT NOT NULL,
    message TEXT NOT NULL,
    related_user_id INTEGER,  -- User who triggered the notification
    related_post_id INTEGER,
    related_comment_id INTEGER,
    related_group_id INTEGER,
    related_session_id INTEGER,
    action_url TEXT,  -- URL to navigate when clicked
    is_read BOOLEAN DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    read_at DATETIME,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (related_user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (related_post_id) REFERENCES posts(id) ON DELETE CASCADE,
    FOREIGN KEY (related_comment_id) REFERENCES comments(id) ON DELETE CASCADE,
    FOREIGN KEY (related_group_id) REFERENCES groups(id) ON DELETE CASCADE
);

CREATE INDEX idx_notifications_user ON notifications(user_id);
CREATE INDEX idx_notifications_read ON notifications(is_read);
CREATE INDEX idx_notifications_created ON notifications(created_at DESC);

-- =============================================================================
-- 2. USER PRESENCE / STATUS SYSTEM
-- =============================================================================

CREATE TABLE IF NOT EXISTS user_presence (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER UNIQUE NOT NULL,
    status TEXT DEFAULT 'offline',  -- 'online', 'away', 'busy', 'offline'
    custom_status TEXT,  -- Custom status message
    last_seen DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_user_presence_status ON user_presence(status);
CREATE INDEX idx_user_presence_last_seen ON user_presence(last_seen DESC);

-- =============================================================================
-- 3. POST BOOKMARKS / SAVED POSTS
-- =============================================================================

CREATE TABLE IF NOT EXISTS post_bookmarks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    post_id INTEGER NOT NULL,
    collection TEXT DEFAULT 'default',  -- Allow organizing bookmarks into collections
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    UNIQUE(user_id, post_id)
);

CREATE INDEX idx_bookmarks_user ON post_bookmarks(user_id);
CREATE INDEX idx_bookmarks_post ON post_bookmarks(post_id);

-- =============================================================================
-- 4. USER SKILLS & INTERESTS
-- =============================================================================

CREATE TABLE IF NOT EXISTS user_skills (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    skill_name TEXT NOT NULL,
    proficiency TEXT DEFAULT 'intermediate',  -- 'beginner', 'intermediate', 'advanced', 'expert'
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(user_id, skill_name)
);

CREATE TABLE IF NOT EXISTS user_interests (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    interest_name TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(user_id, interest_name)
);

CREATE INDEX idx_skills_user ON user_skills(user_id);
CREATE INDEX idx_interests_user ON user_interests(user_id);

-- =============================================================================
-- 5. HASHTAGS & MENTIONS
-- =============================================================================

CREATE TABLE IF NOT EXISTS hashtags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    tag TEXT UNIQUE NOT NULL,
    usage_count INTEGER DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_used_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS post_hashtags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id INTEGER NOT NULL,
    hashtag_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    FOREIGN KEY (hashtag_id) REFERENCES hashtags(id) ON DELETE CASCADE,
    UNIQUE(post_id, hashtag_id)
);

CREATE TABLE IF NOT EXISTS post_mentions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(post_id, user_id)
);

CREATE INDEX idx_hashtags_tag ON hashtags(tag);
CREATE INDEX idx_hashtags_usage ON hashtags(usage_count DESC);
CREATE INDEX idx_post_hashtags_post ON post_hashtags(post_id);
CREATE INDEX idx_post_hashtags_hashtag ON post_hashtags(hashtag_id);
CREATE INDEX idx_post_mentions_user ON post_mentions(user_id);

-- =============================================================================
-- 6. STUDY SESSIONS (Group Scheduling)
-- =============================================================================

CREATE TABLE IF NOT EXISTS study_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    group_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    location TEXT,  -- Physical location or 'online'
    voice_channel_id INTEGER,  -- Link to voice channel if online
    start_time DATETIME NOT NULL,
    end_time DATETIME NOT NULL,
    created_by INTEGER NOT NULL,
    max_participants INTEGER,  -- NULL for unlimited
    is_recurring BOOLEAN DEFAULT 0,
    recurrence_pattern TEXT,  -- 'daily', 'weekly', 'biweekly', etc.
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
    FOREIGN KEY (voice_channel_id) REFERENCES voice_channels(id) ON DELETE SET NULL,
    FOREIGN KEY (created_by) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS session_participants (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    status TEXT DEFAULT 'going',  -- 'going', 'maybe', 'not_going'
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (session_id) REFERENCES study_sessions(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(session_id, user_id)
);

CREATE INDEX idx_sessions_group ON study_sessions(group_id);
CREATE INDEX idx_sessions_start_time ON study_sessions(start_time);
CREATE INDEX idx_participants_session ON session_participants(session_id);
CREATE INDEX idx_participants_user ON session_participants(user_id);

-- =============================================================================
-- 7. GROUP ANNOUNCEMENTS
-- =============================================================================

CREATE TABLE IF NOT EXISTS group_announcements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    group_id INTEGER NOT NULL,
    author_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    content TEXT NOT NULL,
    is_pinned BOOLEAN DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
    FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_announcements_group ON group_announcements(group_id);
CREATE INDEX idx_announcements_pinned ON group_announcements(is_pinned DESC, created_at DESC);

-- =============================================================================
-- 8. USER ANALYTICS & ENGAGEMENT
-- =============================================================================

CREATE TABLE IF NOT EXISTS user_analytics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    metric_type TEXT NOT NULL,  -- 'post_created', 'comment_created', 'login', 'message_sent', etc.
    metric_value INTEGER DEFAULT 1,
    metadata TEXT,  -- JSON for additional context
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS daily_stats (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    date DATE NOT NULL,
    posts_created INTEGER DEFAULT 0,
    comments_created INTEGER DEFAULT 0,
    messages_sent INTEGER DEFAULT 0,
    reactions_given INTEGER DEFAULT 0,
    reactions_received INTEGER DEFAULT 0,
    friends_added INTEGER DEFAULT 0,
    study_sessions_attended INTEGER DEFAULT 0,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(user_id, date)
);

CREATE INDEX idx_analytics_user ON user_analytics(user_id);
CREATE INDEX idx_analytics_type ON user_analytics(metric_type);
CREATE INDEX idx_analytics_created ON user_analytics(created_at);
CREATE INDEX idx_daily_stats_user ON daily_stats(user_id);
CREATE INDEX idx_daily_stats_date ON daily_stats(date DESC);

-- =============================================================================
-- 9. SEARCH INDEX (For full-text search)
-- =============================================================================

-- Virtual table for full-text search on posts
CREATE VIRTUAL TABLE IF NOT EXISTS posts_fts USING fts5(
    content,
    content=posts,
    content_rowid=id
);

-- Triggers to keep FTS index synchronized
CREATE TRIGGER IF NOT EXISTS posts_fts_insert AFTER INSERT ON posts BEGIN
    INSERT INTO posts_fts(rowid, content) VALUES (new.id, new.content);
END;

CREATE TRIGGER IF NOT EXISTS posts_fts_update AFTER UPDATE ON posts BEGIN
    UPDATE posts_fts SET content = new.content WHERE rowid = old.id;
END;

CREATE TRIGGER IF NOT EXISTS posts_fts_delete AFTER DELETE ON posts BEGIN
    DELETE FROM posts_fts WHERE rowid = old.id;
END;

-- Virtual table for user search
CREATE VIRTUAL TABLE IF NOT EXISTS users_fts USING fts5(
    username,
    full_name,
    bio,
    content=users,
    content_rowid=id
);

CREATE TRIGGER IF NOT EXISTS users_fts_insert AFTER INSERT ON users BEGIN
    INSERT INTO users_fts(rowid, username, full_name, bio)
    VALUES (new.id, new.username, COALESCE(new.full_name, ''), COALESCE(new.bio, ''));
END;

CREATE TRIGGER IF NOT EXISTS users_fts_update AFTER UPDATE ON users BEGIN
    UPDATE users_fts SET username = new.username, full_name = COALESCE(new.full_name, ''),
                         bio = COALESCE(new.bio, '')
    WHERE rowid = old.id;
END;

CREATE TRIGGER IF NOT EXISTS users_fts_delete AFTER DELETE ON users BEGIN
    DELETE FROM users_fts WHERE rowid = old.id;
END;

-- =============================================================================
-- 10. ALTER EXISTING TABLES (Add new columns)
-- =============================================================================

-- Add bio and additional profile fields to users table
-- Note: SQLite doesn't support IF NOT EXISTS for ALTER TABLE
-- These will fail gracefully if columns already exist

-- We'll handle this by checking if columns exist first using a safer approach
-- For now, we'll document the desired schema additions:
-- ALTER TABLE users ADD COLUMN bio TEXT;
-- ALTER TABLE users ADD COLUMN website TEXT;
-- ALTER TABLE users ADD COLUMN github_url TEXT;
-- ALTER TABLE users ADD COLUMN linkedin_url TEXT;
-- ALTER TABLE users ADD COLUMN location TEXT;

-- Add is_announcement flag to posts for important posts
-- ALTER TABLE posts ADD COLUMN is_announcement BOOLEAN DEFAULT 0;
-- ALTER TABLE posts ADD COLUMN is_pinned BOOLEAN DEFAULT 0;

-- =============================================================================
-- MIGRATION COMPLETE
-- =============================================================================
