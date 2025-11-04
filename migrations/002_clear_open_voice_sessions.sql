-- Migration: Clear all open voice sessions
-- This migration closes all currently open voice sessions by setting their left_at timestamp

UPDATE voice_sessions
SET left_at = CURRENT_TIMESTAMP
WHERE left_at IS NULL;
