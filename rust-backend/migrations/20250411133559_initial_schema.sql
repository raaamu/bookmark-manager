-- migrations/YYYYMMDDHHMMSS_initial_schema.sql

-- Enable foreign key support if not enabled by default connection string
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL COLLATE NOCASE, -- Ensure unique usernames, case-insensitive
    password_hash TEXT NOT NULL
    -- Add columns for rate limiting if needed:
    -- failed_attempts INTEGER DEFAULT 0 NOT NULL,
    -- lockout_until TEXT -- Store as ISO8601 string or UNIX timestamp
);

-- Optional: Index on username for faster lookups
CREATE UNIQUE INDEX IF NOT EXISTS idx_users_username_lower ON users (LOWER(username));

CREATE TABLE IF NOT EXISTS bookmarks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    url TEXT NOT NULL,
    description TEXT,
    -- Store timestamps as TEXT in ISO8601 format (recommended for SQLite)
    created_at TEXT DEFAULT (strftime('%Y-%m-%d %H:%M:%S', 'now')) NOT NULL,
    updated_at TEXT DEFAULT (strftime('%Y-%m-%d %H:%M:%S', 'now')) NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE, -- Delete bookmarks when user is deleted
    UNIQUE(user_id, url) -- Ensure a user cannot add the same URL twice
);

-- Index for faster user bookmark lookups
CREATE INDEX IF NOT EXISTS idx_bookmarks_user_id ON bookmarks (user_id);
-- Optional: Index for URL lookups within a user
CREATE INDEX IF NOT EXISTS idx_bookmarks_user_url ON bookmarks (user_id, url);

-- Trigger to update 'updated_at' timestamp automatically on bookmark update
CREATE TRIGGER IF NOT EXISTS trigger_bookmarks_update_updated_at
AFTER UPDATE ON bookmarks
FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at -- Avoid infinite loop if updated_at is explicitly set
BEGIN
    UPDATE bookmarks
    SET updated_at = strftime('%Y-%m-%d %H:%M:%S', 'now')
    WHERE id = OLD.id;
END;
-- Add migration script here
