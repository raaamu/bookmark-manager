use axum::{
    http::StatusCode,
    response::{IntoResponse, Response},
    Json,
};
use serde_json::json;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum AppError {
    #[error("Database error: {0}")]
    SqlxError(#[from] sqlx::Error),

    #[error("Migration error: {0}")]
    MigrateError(#[from] sqlx::migrate::MigrateError),

    #[error("Configuration error: {0}")]
    ConfigError(#[from] crate::config::ConfigError),

    #[error("Password hashing error: {0}")]
    PasswordHashingError(argon2::Error),

    #[error("Password hash parsing error: {0}")]
    PasswordHashParseError(argon2::password_hash::Error),

    #[error("JWT error: {0}")]
    JwtError(#[from] jsonwebtoken::errors::Error),

    #[error("Invalid input: {0}")]
    InvalidInput(String),

    #[error("User already exists")]
    UserAlreadyExists,

    #[error("Invalid credentials")]
    InvalidCredentials,

    #[error("User not found")]
    UserNotFound,

    #[error("Bookmark already exists")]
    BookmarkAlreadyExists,

    #[error("Bookmark not found")]
    BookmarkNotFound,

    #[error("Unauthorized")]
    Unauthorized,

    #[error("URL parsing error: {0}")]
    UrlParseError(#[from] url::ParseError),

    #[error("Internal server error: {0}")]
    InternalServerError(String), // Add context for internal errors
}

impl From<base64::DecodeError> for AppError {
    fn from(err: base64::DecodeError) -> Self {
        AppError::InternalServerError(format!("Base64 decode error: {}", err))
    }
}

// Map AppError to HTTP responses
impl IntoResponse for AppError {
    fn into_response(self) -> Response {
        let (status, error_message) = match self {
            AppError::SqlxError(e) => {
                eprintln!("SQLx error: {:?}", e); // Log the detailed error
                (StatusCode::INTERNAL_SERVER_ERROR, "Database error".to_string())
            }
            AppError::MigrateError(e) => {
                 eprintln!("Migration error: {:?}", e);
                (StatusCode::INTERNAL_SERVER_ERROR, "Database migration failed".to_string())
            }
            AppError::ConfigError(e) => {
                 eprintln!("Config error: {:?}", e);
                (StatusCode::INTERNAL_SERVER_ERROR, "Configuration error".to_string())
            }
            AppError::PasswordHashingError(e) => {
                 eprintln!("Password hashing error: {:?}", e);
                (StatusCode::INTERNAL_SERVER_ERROR, "Internal server error (hash)".to_string())
            }
             AppError::PasswordHashParseError(e) => {
                 eprintln!("Password hash parse error: {:?}", e);
                 (StatusCode::INTERNAL_SERVER_ERROR, "Internal server error (hash parse)".to_string())
             }
             AppError::JwtError(e) => {
                 eprintln!("JWT error: {:?}", e);
                 (StatusCode::INTERNAL_SERVER_ERROR, "Token generation/validation error".to_string())
             }
            AppError::InvalidInput(msg) => (StatusCode::BAD_REQUEST, msg),
            AppError::UserAlreadyExists => (StatusCode::CONFLICT, "User already exists".to_string()),
            AppError::InvalidCredentials => (StatusCode::UNAUTHORIZED, "Invalid username or password".to_string()),
            AppError::UserNotFound => (StatusCode::NOT_FOUND, "User not found".to_string()),
            AppError::BookmarkAlreadyExists => (StatusCode::CONFLICT, "Bookmark with this URL already exists".to_string()),
            AppError::BookmarkNotFound => (StatusCode::NOT_FOUND, "Bookmark not found".to_string()),
            AppError::Unauthorized => (StatusCode::UNAUTHORIZED, "Unauthorized".to_string()),
            AppError::UrlParseError(_) => (StatusCode::BAD_REQUEST, "Invalid URL format".to_string()),
            AppError::InternalServerError(msg) => {
                eprintln!("Internal server error: {}", msg);
                (StatusCode::INTERNAL_SERVER_ERROR, "Internal server error".to_string())
            },
        };

        let body = Json(json!({ "error": error_message }));
        (status, body).into_response()
    }
}

// Convenience type alias
pub type Result<T> = std::result::Result<T, AppError>;
