-- Migration: Create Users Table
-- Version: 000
-- Description: Creates the core users table for authentication and profiles

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    name TEXT,
    position TEXT,
    phone_number TEXT,
    university TEXT,
    department TEXT,
    enrollment_year INTEGER,
    warnings INTEGER DEFAULT 0,
    primary_language TEXT,
    additional_languages TEXT,
    role TEXT DEFAULT 'Student',
    avatar_url TEXT,
    banner_url TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Create indexes for faster lookups
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
