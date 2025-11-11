-- Study Buddy Matching System Migration
-- Version: 005
-- Description: Adds tables for intelligent study partner matching

-- Study Preferences Table
-- Stores user preferences for finding compatible study partners
CREATE TABLE IF NOT EXISTS study_preferences (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,

    -- Learning preferences
    learning_style TEXT CHECK(learning_style IN ('visual', 'auditory', 'reading_writing', 'kinesthetic', 'mixed')) DEFAULT 'mixed',
    study_environment TEXT CHECK(study_environment IN ('quiet', 'moderate', 'lively', 'flexible')) DEFAULT 'flexible',
    study_time_preference TEXT CHECK(study_time_preference IN ('early_morning', 'morning', 'afternoon', 'evening', 'night', 'flexible')) DEFAULT 'flexible',

    -- Academic focus
    courses TEXT, -- JSON array of course names/codes
    topics_of_interest TEXT, -- JSON array of topics
    academic_goals TEXT, -- Short description of goals

    -- Availability
    available_days TEXT, -- JSON array: ["monday", "tuesday", ...]
    available_hours_per_week INTEGER DEFAULT 5,

    -- Matching preferences
    preferred_group_size INTEGER CHECK(preferred_group_size BETWEEN 1 AND 10) DEFAULT 2,
    same_university_only BOOLEAN DEFAULT TRUE,
    same_department_only BOOLEAN DEFAULT FALSE,
    same_year_only BOOLEAN DEFAULT FALSE,

    -- Metadata
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_study_prefs_user ON study_preferences(user_id);
CREATE INDEX IF NOT EXISTS idx_study_prefs_active ON study_preferences(is_active);

-- Study Buddy Matches Table
-- Stores calculated compatibility matches between users
CREATE TABLE IF NOT EXISTS study_buddy_matches (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    matched_user_id BIGINT NOT NULL,

    -- Compatibility scoring (0-100)
    compatibility_score REAL NOT NULL,
    course_overlap_score REAL DEFAULT 0,
    schedule_compatibility_score REAL DEFAULT 0,
    learning_style_score REAL DEFAULT 0,
    academic_level_score REAL DEFAULT 0,

    -- Match metadata
    common_courses TEXT, -- JSON array of common courses
    common_interests TEXT, -- JSON array of common topics
    match_reason TEXT, -- Brief explanation of why they're matched

    -- Match status
    status TEXT CHECK(status IN ('suggested', 'accepted', 'declined', 'blocked')) DEFAULT 'suggested',
    viewed_at TIMESTAMP,
    responded_at TIMESTAMP,

    -- Metadata
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (matched_user_id) REFERENCES users(id) ON DELETE CASCADE,

    UNIQUE(user_id, matched_user_id)
);

CREATE INDEX IF NOT EXISTS idx_matches_user ON study_buddy_matches(user_id, status);
CREATE INDEX IF NOT EXISTS idx_matches_score ON study_buddy_matches(compatibility_score DESC);
CREATE INDEX IF NOT EXISTS idx_matches_status ON study_buddy_matches(status);

-- Study Buddy Connections Table
-- Tracks confirmed study buddy relationships
CREATE TABLE IF NOT EXISTS study_buddy_connections (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    buddy_id BIGINT NOT NULL,

    -- Connection details
    connected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_study_session TIMESTAMP,
    total_study_sessions INTEGER DEFAULT 0,
    connection_strength INTEGER DEFAULT 0, -- 0-100, increases with interactions

    -- Connection preferences
    is_favorite BOOLEAN DEFAULT FALSE,
    notification_enabled BOOLEAN DEFAULT TRUE,

    -- Metadata
    notes TEXT, -- Private notes about this study buddy
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (buddy_id) REFERENCES users(id) ON DELETE CASCADE,

    UNIQUE(user_id, buddy_id)
);

CREATE INDEX IF NOT EXISTS idx_connections_user ON study_buddy_connections(user_id);
CREATE INDEX IF NOT EXISTS idx_connections_strength ON study_buddy_connections(connection_strength DESC);
CREATE INDEX IF NOT EXISTS idx_connections_favorite ON study_buddy_connections(is_favorite);

-- Study Session Plans Table
-- Allows study buddies to schedule and track study sessions
CREATE TABLE IF NOT EXISTS study_session_plans (
    id BIGSERIAL PRIMARY KEY,
    creator_id BIGINT NOT NULL,

    -- Session details
    title TEXT NOT NULL,
    description TEXT,
    course_topic TEXT,
    session_type TEXT CHECK(session_type IN ('individual', 'pair', 'small_group', 'large_group')) DEFAULT 'pair',

    -- Scheduling
    scheduled_start TIMESTAMP NOT NULL,
    scheduled_end TIMESTAMP NOT NULL,
    location TEXT, -- Physical location or "online"
    meeting_link TEXT, -- For online sessions

    -- Participants
    max_participants INTEGER DEFAULT 5,
    current_participants INTEGER DEFAULT 1,

    -- Status
    status TEXT CHECK(status IN ('planned', 'in_progress', 'completed', 'cancelled')) DEFAULT 'planned',
    completion_rating INTEGER CHECK(completion_rating BETWEEN 1 AND 5),
    completion_notes TEXT,

    -- Metadata
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (creator_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_sessions_creator ON study_session_plans(creator_id);
CREATE INDEX IF NOT EXISTS idx_sessions_status ON study_session_plans(status);
CREATE INDEX IF NOT EXISTS idx_sessions_scheduled ON study_session_plans(scheduled_start);

-- Study Session Participants Table
-- Links users to study sessions
CREATE TABLE IF NOT EXISTS study_session_participants (
    id BIGSERIAL PRIMARY KEY,
    session_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,

    -- Participation
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    attendance_status TEXT CHECK(attendance_status IN ('registered', 'attended', 'missed', 'cancelled')) DEFAULT 'registered',

    -- Feedback
    rating INTEGER CHECK(rating BETWEEN 1 AND 5),
    feedback TEXT,

    FOREIGN KEY (session_id) REFERENCES study_session_plans(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,

    UNIQUE(session_id, user_id)
);

CREATE INDEX IF NOT EXISTS idx_session_parts_session ON study_session_participants(session_id);
CREATE INDEX IF NOT EXISTS idx_session_parts_user ON study_session_participants(user_id);

-- =============================================================================
-- PostgreSQL Triggers for updated_at timestamps
-- =============================================================================

-- Function to update timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Trigger for study_preferences
DROP TRIGGER IF EXISTS update_study_preferences_timestamp ON study_preferences;
CREATE TRIGGER update_study_preferences_timestamp
    BEFORE UPDATE ON study_preferences
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Trigger for study_buddy_matches
DROP TRIGGER IF EXISTS update_study_matches_timestamp ON study_buddy_matches;
CREATE TRIGGER update_study_matches_timestamp
    BEFORE UPDATE ON study_buddy_matches
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Trigger for study_buddy_connections
DROP TRIGGER IF EXISTS update_study_connections_timestamp ON study_buddy_connections;
CREATE TRIGGER update_study_connections_timestamp
    BEFORE UPDATE ON study_buddy_connections
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Trigger for study_session_plans
DROP TRIGGER IF EXISTS update_study_sessions_timestamp ON study_session_plans;
CREATE TRIGGER update_study_sessions_timestamp
    BEFORE UPDATE ON study_session_plans
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();
