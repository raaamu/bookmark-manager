use axum::{
    extract::{Path, Query, State},
    http::StatusCode,
    response::IntoResponse,
    Json,
};
use crate::{
    auth,
    db,
    errors::{AppError, Result},
    models::{LoginResponse, NewBookmark, UserLogin, UserRegistration, Bookmark, AuthenticatedUser}, // Import AuthenticatedUser model
    AppState,
};
use serde::Deserialize;
use url::Url; // For URL validation

// --- Placeholder Authentication ---
// Replace this with real JWT middleware or extractor
fn authenticate_stub(user_id: i64, username: &str) -> AuthenticatedUser {
    println!("WARNING: Using placeholder authentication for user_id: {}, username: {}", user_id, username);
    AuthenticatedUser {
        user_id,
        username: username.to_string(),
    }
}
// --- End Placeholder ---


// --- User Handlers ---

pub async fn register_user(
    State(state): State<AppState>,
    Json(payload): Json<UserRegistration>,
) -> Result<impl IntoResponse> {
    // Validate input
    if payload.username.trim().is_empty() || payload.password.is_empty() {
        return Err(AppError::InvalidInput("Username and password cannot be empty".to_string()));
    }
     // Basic username validation (example)
    if payload.username.len() < 3 || payload.username.len() > 50 {
         return Err(AppError::InvalidInput("Username must be between 3 and 50 characters".to_string()));
    }
    if !auth::is_password_strong(&payload.password) {
         return Err(AppError::InvalidInput(
             "Password is too weak. Requires minimum 8 chars, and 3 of: uppercase, lowercase, digit, special char.".to_string()
         ));
    }

    // Hash password
    let password_hash = auth::hash_password(&payload.password, &state.config)?;

    // Create user in DB
    let user_id = db::create_user(&state.db_pool, &payload.username.trim(), &password_hash).await?;

    // Optionally: Log the user in immediately and return a token
    // let token = auth::create_jwt(user_id, &payload.username, &state.config)?;
    // Ok((StatusCode::CREATED, Json(LoginResponse { token })))

    Ok((StatusCode::CREATED, Json(serde_json::json!({ "message": "User created successfully", "user_id": user_id }))))
}

pub async fn login_user(
    State(state): State<AppState>,
    Json(payload): Json<UserLogin>,
) -> Result<Json<LoginResponse>> {
     if payload.username.trim().is_empty() || payload.password.is_empty() {
        return Err(AppError::InvalidInput("Username and password cannot be empty".to_string()));
    }

    // Find user (case-insensitive recommended)
    let username_trimmed = payload.username.trim();
    let user = db::find_user_by_username(&state.db_pool, username_trimmed)
        .await?
        // Use InvalidCredentials for both not found and wrong password for security
        .ok_or(AppError::InvalidCredentials)?;

    // Verify password
    if !auth::verify_password(&user.password_hash, &payload.password)? {
        // TODO: Implement rate limiting / account lockout here based on failed attempts
        // This might involve updating the user record in the DB
        return Err(AppError::InvalidCredentials);
    }

    // TODO: Reset failed attempts counter in DB on successful login if implementing DB rate limiting

    // Create JWT
    let token = auth::create_jwt(user.id, &user.username, &state.config)?;

    Ok(Json(LoginResponse { token }))
}

// --- Bookmark Handlers ---
// These handlers should require authentication.
// Use the AuthenticatedUser extractor or middleware.
// The `authenticate_stub` is a TEMPORARY placeholder.

pub async fn add_bookmark(
    State(state): State<AppState>,
    // auth_user: AuthenticatedUser, // Use this when extractor is implemented
    Json(payload): Json<NewBookmark>,
) -> Result<impl IntoResponse> {
    // --- TODO: Replace this placeholder with real authentication ---
    let auth_user = authenticate_stub(1, "testuser"); // Example placeholder
    // --- End placeholder ---


    // Validate input
    if payload.title.trim().is_empty() || payload.url.trim().is_empty() {
         return Err(AppError::InvalidInput("Title and URL cannot be empty".to_string()));
    }
    // Validate URL format
    let parsed_url = Url::parse(payload.url.trim())?; // This implicitly uses AppError::UrlParseError

    // Use the validated & trimmed URL
    let bookmark_to_add = NewBookmark {
        title: payload.title.trim().to_string(),
        url: parsed_url.to_string(), // Use the parsed URL string
        description: payload.description.map(|d| d.trim().to_string()).filter(|d| !d.is_empty()), // Trim and filter empty description
    };


    let bookmark_id = db::add_bookmark(&state.db_pool, auth_user.user_id, &bookmark_to_add).await?;

    // Fetch the created bookmark to return it (optional, but good practice)
    // let created_bookmark = db::get_bookmark_by_id(&state.db_pool, bookmark_id).await?;
    // Ok((StatusCode::CREATED, Json(created_bookmark)))

     Ok((StatusCode::CREATED, Json(serde_json::json!({ "message": "Bookmark added", "id": bookmark_id }))))
}

pub async fn get_bookmarks(
    State(state): State<AppState>,
    // auth_user: AuthenticatedUser, // Use this when extractor is implemented
) -> Result<Json<Vec<Bookmark>>> {
    // --- TODO: Replace this placeholder with real authentication ---
    let auth_user = authenticate_stub(1, "testuser"); // Example placeholder
    // --- End placeholder ---

    let bookmarks = db::get_user_bookmarks(&state.db_pool, auth_user.user_id).await?;
    Ok(Json(bookmarks))
}

#[derive(Deserialize, Debug)]
pub struct SearchQuery {
    q: Option<String>, // Make query optional
}

pub async fn search_bookmarks(
    State(state): State<AppState>,
    // auth_user: AuthenticatedUser, // Use this when extractor is implemented
    Query(query): Query<SearchQuery>,
) -> Result<Json<Vec<Bookmark>>> {
    // --- TODO: Replace this placeholder with real authentication ---
    let auth_user = authenticate_stub(1, "testuser"); // Example placeholder
    // --- End placeholder ---

    // Handle potentially missing query parameter gracefully
    let search_term = query.q.unwrap_or_default(); // Use empty string if 'q' is missing
    if search_term.trim().is_empty() {
        // Return all bookmarks for the user if query is empty/missing, or return error/empty list
         return db::get_user_bookmarks(&state.db_pool, auth_user.user_id).await.map(Json);
        // Alternatively: return Err(AppError::InvalidInput("Search query cannot be empty".to_string()));
        // Alternatively: return Ok(Json(vec![]));
    }

    let bookmarks = db::search_user_bookmarks(&state.db_pool, auth_user.user_id, search_term.trim()).await?;
    Ok(Json(bookmarks))
}

pub async fn delete_bookmark(
    State(state): State<AppState>,
    // auth_user: AuthenticatedUser, // Use this when extractor is implemented
    Path(id): Path<i64>,
) -> Result<StatusCode> {
     // --- TODO: Replace this placeholder with real authentication ---
    let auth_user = authenticate_stub(1, "testuser"); // Example placeholder
    // --- End placeholder ---

    if id <= 0 {
        return Err(AppError::InvalidInput("Invalid bookmark ID".to_string()));
    }

    db::delete_bookmark(&state.db_pool, auth_user.user_id, id).await?;
    Ok(StatusCode::NO_CONTENT) // 204 No Content is standard for successful DELETE
}
