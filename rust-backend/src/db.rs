use crate::errors::{AppError, Result};
use crate::models::{Bookmark, NewBookmark, User};
use sqlx::SqlitePool;

// --- User Functions ---

pub async fn find_user_by_username(pool: &SqlitePool, username: &str) -> Result<Option<User>> {
    // Adjust the SELECT query based on the actual User struct fields
    let user = sqlx::query_as::<_, User>(
        "SELECT id, username, password_hash FROM users WHERE username = ?",
    )
    .bind(username)
    .fetch_optional(pool)
    .await?;
    Ok(user)
}

pub async fn create_user(pool: &SqlitePool, username: &str, password_hash: &str) -> Result<i64> {
    // Check if user already exists (case-insensitive check recommended)
    let existing_user = sqlx::query!("SELECT id FROM users WHERE LOWER(username) = LOWER(?)", username)
        .fetch_optional(pool)
        .await?;

    if existing_user.is_some() {
        return Err(AppError::UserAlreadyExists);
    }

    let result = sqlx::query!(
        "INSERT INTO users (username, password_hash) VALUES (?, ?)",
        username,
        password_hash
    )
    .execute(pool)
    .await?;

    Ok(result.last_insert_rowid())
}

// --- Bookmark Functions ---

pub async fn add_bookmark(
    pool: &SqlitePool,
    user_id: i64,
    new_bookmark: &NewBookmark,
) -> Result<i64> {
    // Check if bookmark with the same URL already exists for this user
    let exists: Option<i32> = sqlx::query_scalar(
        "SELECT 1 FROM bookmarks WHERE user_id = ? AND url = ?",
    )
    .bind(user_id)
    .bind(&new_bookmark.url)
    .fetch_optional(pool)
    .await?;

    if exists.is_some() {
        return Err(AppError::BookmarkAlreadyExists);
    }

    let description = new_bookmark.description.as_deref(); // Keep as Option<&str>

    let result = sqlx::query!(
        "INSERT INTO bookmarks (user_id, title, url, description) VALUES (?, ?, ?, ?)",
        user_id,
        new_bookmark.title,
        new_bookmark.url,
        description // Pass Option<&str> directly
    )
    .execute(pool)
    .await?;

    Ok(result.last_insert_rowid())
}

pub async fn get_user_bookmarks(pool: &SqlitePool, user_id: i64) -> Result<Vec<Bookmark>> {
    let bookmarks = sqlx::query_as::<_, Bookmark>(
        r#"
        SELECT
            id,
            user_id,
            title,
            url,
            description,
            created_at,
            updated_at
        FROM bookmarks
        WHERE user_id = ?
        ORDER BY created_at DESC
        "#,
    )
    .bind(user_id)
    .fetch_all(pool)
    .await?;
    Ok(bookmarks)
}

pub async fn search_user_bookmarks(
    pool: &SqlitePool,
    user_id: i64,
    query: &str,
) -> Result<Vec<Bookmark>> {
    let search_term = format!("%{}%", query); // Keep case for LIKE if DB collation handles it, or use LOWER()

    // Use LOWER() in SQL for case-insensitive search
    let bookmarks = sqlx::query_as::<_, Bookmark>(
        r#"
        SELECT
            id,
            user_id,
            title,
            url,
            description,
            created_at,
            updated_at
        FROM bookmarks
        WHERE user_id = ?
          AND (
            LOWER(title) LIKE LOWER(?) OR
            LOWER(url) LIKE LOWER(?) OR
            LOWER(description) LIKE LOWER(?)
          )
        ORDER BY created_at DESC
        "#,
    )
    .bind(user_id)
    .bind(&search_term)
    .bind(&search_term)
    .bind(&search_term)
    .fetch_all(pool)
    .await?;
    Ok(bookmarks)
}


pub async fn delete_bookmark(pool: &SqlitePool, user_id: i64, bookmark_id: i64) -> Result<()> {
    let result = sqlx::query!(
        "DELETE FROM bookmarks WHERE id = ? AND user_id = ?",
        bookmark_id,
        user_id
    )
    .execute(pool)
    .await?;

    if result.rows_affected() == 0 {
        // Could also check if the bookmark exists at all first to differentiate
        // between "not found" and "not authorized to delete".
        Err(AppError::BookmarkNotFound)
    } else {
        Ok(())
    }
}
