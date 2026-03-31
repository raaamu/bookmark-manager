use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use sqlx::FromRow;

#[derive(Debug, Serialize, Deserialize, FromRow, Clone)] // Add Clone
pub struct User {
    pub id: i64, // Use i64 for SQLite INTEGER PRIMARY KEY
    pub username: String,
    #[serde(skip_serializing)] // Don't send password hash to client
    pub password_hash: String,
    // Add failed_attempts and last_attempt if implementing DB-based rate limiting
}

#[derive(Debug, Serialize, Deserialize)]
pub struct UserRegistration {
    pub username: String,
    pub password: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct UserLogin {
    pub username: String,
    pub password: String,
}

#[derive(Debug, Serialize, Deserialize, FromRow, Clone)] // Add Clone
pub struct Bookmark {
    pub id: i64,
    pub user_id: i64,
    pub title: String,
    pub url: String,
    pub description: Option<String>, // Use Option for nullable fields
    pub created_at: Option<DateTime<Utc>>,
    pub updated_at: Option<DateTime<Utc>>,
}

#[derive(Debug, Deserialize)]
pub struct NewBookmark {
    pub title: String,
    pub url: String,
    pub description: Option<String>,
}

#[derive(Debug, Serialize)]
pub struct LoginResponse {
    pub token: String,
}

// Claims for JWT
#[derive(Debug, Serialize, Deserialize)]
pub struct Claims {
    pub sub: String, // Subject (username)
    pub user_id: i64,
    pub exp: usize,  // Expiration time (as timestamp)
    pub iat: usize,  // Issued at time (as timestamp)
}

// --- Structs for Authentication Extractor ---
// (If you implement the extractor pattern)
#[derive(Debug, Clone)]
pub struct AuthenticatedUser {
    pub user_id: i64,
    pub username: String,
}
