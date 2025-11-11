-- Migration: Enhanced Features for Sohbet Academic Platform
-- Date: November 5, 2025
-- Description: Adds notifications, user presence, bookmarks, skills/interests,
--              hashtags, mentions, study sessions, and analytics

-- =============================================================================
-- 1. NOTIFICATIONS SYSTEM
-- =============================================================================

CREATE TABLE IF NOT EXISTS notifications (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    type TEXT NOT NULL,  -- 'friend_request', 'friend_accepted', 'post_like', 'post_comment',
                         -- 'mention', 'group_invite', 'study_session', 'announcement'
    title TEXT NOT NULL,
    message TEXT NOT NULL,
    related_user_id BIGINT,  -- User who triggered the notification
    related_post_id BIGINT,
    related_comment_id BIGINT,
    related_group_id BIGINT,
    related_session_id BIGINT,
    action_url TEXT,  -- URL to navigate when clicked
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    read_at TIMESTAMP,
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
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT UNIQUE NOT NULL,
    status TEXT DEFAULT 'offline',  -- 'online', 'away', 'busy', 'offline'
    custom_status TEXT,  -- Custom status message
    last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_user_presence_status ON user_presence(status);
CREATE INDEX idx_user_presence_last_seen ON user_presence(last_seen DESC);

-- =============================================================================
-- 3. POST BOOKMARKS / SAVED POSTS
-- =============================================================================

CREATE TABLE IF NOT EXISTS post_bookmarks (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    post_id BIGINT NOT NULL,
    collection TEXT DEFAULT 'default',  -- Allow organizing bookmarks into collections
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
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
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    skill_name TEXT NOT NULL,
    proficiency TEXT DEFAULT 'intermediate',  -- 'beginner', 'intermediate', 'advanced', 'expert'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(user_id, skill_name)
);

CREATE TABLE IF NOT EXISTS user_interests (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    interest_name TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE(user_id, interest_name)
);

CREATE INDEX idx_skills_user ON user_skills(user_id);
CREATE INDEX idx_interests_user ON user_interests(user_id);

-- =============================================================================
-- 5. HASHTAGS & MENTIONS
-- =============================================================================

CREATE TABLE IF NOT EXISTS hashtags (
    id BIGSERIAL PRIMARY KEY,
    tag TEXT UNIQUE NOT NULL,
    usage_count INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_used_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS post_hashtags (
    id BIGSERIAL PRIMARY KEY,
    post_id BIGINT NOT NULL,
    hashtag_id BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    FOREIGN KEY (hashtag_id) REFERENCES hashtags(id) ON DELETE CASCADE,
    UNIQUE(post_id, hashtag_id)
);

CREATE TABLE IF NOT EXISTS post_mentions (
    id BIGSERIAL PRIMARY KEY,
    post_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
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
    id BIGSERIAL PRIMARY KEY,
    group_id BIGINT NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    location TEXT,  -- Physical location or 'online'
    voice_channel_id BIGINT,  -- Link to voice channel if online
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP NOT NULL,
    created_by BIGINT NOT NULL,
    max_participants INTEGER,  -- NULL for unlimited
    is_recurring BOOLEAN DEFAULT FALSE,
    recurrence_pattern TEXT,  -- 'daily', 'weekly', 'biweekly', etc.
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
    FOREIGN KEY (voice_channel_id) REFERENCES voice_channels(id) ON DELETE SET NULL,
    FOREIGN KEY (created_by) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS session_participants (
    id BIGSERIAL PRIMARY KEY,
    session_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    status TEXT DEFAULT 'going',  -- 'going', 'maybe', 'not_going'
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
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
    id BIGSERIAL PRIMARY KEY,
    group_id BIGINT NOT NULL,
    author_id BIGINT NOT NULL,
    title TEXT NOT NULL,
    content TEXT NOT NULL,
    is_pinned BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
    FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_announcements_group ON group_announcements(group_id);
CREATE INDEX idx_announcements_pinned ON group_announcements(is_pinned DESC, created_at DESC);

-- =============================================================================
-- 8. USER ANALYTICS & ENGAGEMENT
-- =============================================================================

CREATE TABLE IF NOT EXISTS user_analytics (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    metric_type TEXT NOT NULL,  -- 'post_created', 'comment_created', 'login', 'message_sent', etc.
    metric_value INTEGER DEFAULT 1,
    metadata TEXT,  -- JSON for additional context
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS daily_stats (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
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
-- 9. FULL-TEXT SEARCH (PostgreSQL Native)
-- =============================================================================

-- Add tsvector columns for full-text search
ALTER TABLE posts ADD COLUMN IF NOT EXISTS content_tsv tsvector;
ALTER TABLE users ADD COLUMN IF NOT EXISTS search_tsv tsvector;

-- Create GIN indexes for full-text search
CREATE INDEX IF NOT EXISTS idx_posts_content_tsv ON posts USING gin(content_tsv);
CREATE INDEX IF NOT EXISTS idx_users_search_tsv ON users USING gin(search_tsv);

-- Function to update posts tsvector
CREATE OR REPLACE FUNCTION posts_search_trigger() RETURNS trigger AS $$
BEGIN
    NEW.content_tsv := to_tsvector('english', COALESCE(NEW.content, ''));
    RETURN NEW;
END
$$ LANGUAGE plpgsql;

-- Function to update users tsvector
CREATE OR REPLACE FUNCTION users_search_trigger() RETURNS trigger AS $$
BEGIN
    NEW.search_tsv := to_tsvector('english',
        COALESCE(NEW.username, '') || ' ' ||
        COALESCE(NEW.name, '') || ' ' ||
        COALESCE(NEW.email, '')
    );
    RETURN NEW;
END
$$ LANGUAGE plpgsql;

-- Triggers for posts
DROP TRIGGER IF EXISTS posts_search_update ON posts;
CREATE TRIGGER posts_search_update
    BEFORE INSERT OR UPDATE ON posts
    FOR EACH ROW
    EXECUTE FUNCTION posts_search_trigger();

-- Triggers for users
DROP TRIGGER IF EXISTS users_search_update ON users;
CREATE TRIGGER users_search_update
    BEFORE INSERT OR UPDATE ON users
    FOR EACH ROW
    EXECUTE FUNCTION users_search_trigger();

-- Update existing rows
UPDATE posts SET content_tsv = to_tsvector('english', COALESCE(content, '')) WHERE content_tsv IS NULL;
UPDATE users SET search_tsv = to_tsvector('english',
    COALESCE(username, '') || ' ' ||
    COALESCE(name, '') || ' ' ||
    COALESCE(email, '')
) WHERE search_tsv IS NULL;

-- =============================================================================
-- 10. ALTER EXISTING TABLES (Add new columns)
-- =============================================================================

-- Add bio and additional profile fields to users table
-- PostgreSQL supports IF NOT EXISTS for ALTER TABLE ADD COLUMN

ALTER TABLE users ADD COLUMN IF NOT EXISTS bio TEXT;
ALTER TABLE users ADD COLUMN IF NOT EXISTS website TEXT;
ALTER TABLE users ADD COLUMN IF NOT EXISTS github_url TEXT;
ALTER TABLE users ADD COLUMN IF NOT EXISTS linkedin_url TEXT;
ALTER TABLE users ADD COLUMN IF NOT EXISTS location TEXT;
ALTER TABLE users ADD COLUMN IF NOT EXISTS full_name TEXT;

-- Add is_announcement flag to posts for important posts
ALTER TABLE posts ADD COLUMN IF NOT EXISTS is_announcement BOOLEAN DEFAULT FALSE;
ALTER TABLE posts ADD COLUMN IF NOT EXISTS is_pinned BOOLEAN DEFAULT FALSE;

-- =============================================================================
-- MIGRATION COMPLETE
-- =============================================================================
