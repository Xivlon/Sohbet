-- Migration: Email Verification System
-- Version: 002
-- Description: Adds email verification tokens and tracking for user email verification

-- Add email_verified column to users table if it doesn't exist
-- We use a safe approach that checks if column exists first
-- Since SQLite doesn't have IF NOT EXISTS for columns, we check with a query first
-- This will be handled by the application code to safely add the column

-- Create email_verification_tokens table
CREATE TABLE IF NOT EXISTS email_verification_tokens (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    token TEXT UNIQUE NOT NULL,
    expires_at INTEGER NOT NULL,  -- Unix timestamp
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    verified_at DATETIME,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Create indexes for faster token lookups
CREATE INDEX IF NOT EXISTS idx_email_verification_tokens_token ON email_verification_tokens(token);
CREATE INDEX IF NOT EXISTS idx_email_verification_tokens_user_id ON email_verification_tokens(user_id);
CREATE INDEX IF NOT EXISTS idx_email_verification_tokens_expires_at ON email_verification_tokens(expires_at);

-- Add email_verified column to users table
-- SQLite requires a default value for new columns in ALTER TABLE
ALTER TABLE users ADD COLUMN email_verified INTEGER DEFAULT 0;
