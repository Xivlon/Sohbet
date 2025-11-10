-- Migration: Email Verification System
-- Version: 002
-- Description: Adds email verification tokens and tracking for user email verification

-- Add email_verified column to users table if it doesn't exist
-- We use a safe approach that checks if column exists first
-- PostgreSQL supports IF NOT EXISTS for ALTER TABLE ADD COLUMN in version 9.6+

-- Create email_verification_tokens table
CREATE TABLE IF NOT EXISTS email_verification_tokens (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    token TEXT UNIQUE NOT NULL,
    expires_at BIGINT NOT NULL,  -- Unix timestamp
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    verified_at TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Create indexes for faster token lookups
CREATE INDEX IF NOT EXISTS idx_email_verification_tokens_token ON email_verification_tokens(token);
CREATE INDEX IF NOT EXISTS idx_email_verification_tokens_user_id ON email_verification_tokens(user_id);
CREATE INDEX IF NOT EXISTS idx_email_verification_tokens_expires_at ON email_verification_tokens(expires_at);

-- Add email_verified column to users table
-- PostgreSQL uses BOOLEAN type with TRUE/FALSE values
ALTER TABLE users ADD COLUMN IF NOT EXISTS email_verified BOOLEAN DEFAULT FALSE;
