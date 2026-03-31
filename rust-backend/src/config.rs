use rand::RngCore;
use std::env;
use thiserror::Error;
use hex; // Make sure hex is imported
use base64;

#[derive(Debug, Error)]
pub enum ConfigError {
    #[error("Missing environment variable: {0}")]
    MissingEnvVar(String),
    #[error("Invalid Base64 salt: {0}")]
    InvalidSaltFormat(#[from] base64::DecodeError), // If using Base64 for salt
    #[error("Invalid Hex salt: {0}")]
    InvalidHexSaltFormat(#[from] hex::FromHexError), // If using Hex for salt
}

#[derive(Clone, Debug)]
pub struct Config {
    pub database_url: String,
    pub jwt_secret: String,
    pub jwt_expires_in_hours: u64,
    pub password_salt: String, // Salt for Argon2 - should be unique per deployment (Hex encoded)
}

impl Config {
    pub async fn from_env() -> Result<Self, ConfigError> {
        let database_url = env::var("DATABASE_URL")
            .map_err(|_| ConfigError::MissingEnvVar("DATABASE_URL".to_string()))?;

        // Generate a secure JWT secret if not provided via env var
        // IN PRODUCTION: Load this from a secure source (env var, secrets manager)
        let jwt_secret = env::var("JWT_SECRET").unwrap_or_else(|_| {
            println!("WARNING: JWT_SECRET not set, generating a random one. Set JWT_SECRET env var for persistent logins.");
            let mut key = [0u8; 64]; // 64 bytes = 512 bits
            rand::thread_rng().fill_bytes(&mut key);
            hex::encode(key) // Use hex encoding
        });

        // Generate a secure salt if not provided via env var (Hex encoded)
        // IN PRODUCTION: Load this from a secure source (env var, secrets manager)
        let password_salt = env::var("PASSWORD_SALT").unwrap_or_else(|_| {
             println!("WARNING: PASSWORD_SALT not set, generating a random one. Set PASSWORD_SALT env var for security (Hex encoded).");
            let mut salt_bytes = [0u8; 16]; // 16 bytes is recommended for Argon2 salt
            rand::thread_rng().fill_bytes(&mut salt_bytes);
            hex::encode(salt_bytes) // Use hex encoding
        });

        // Validate the loaded/generated salt format (optional but good)
        hex::decode(&password_salt)?; // Check if it's valid hex

        let jwt_expires_in_hours = env::var("JWT_EXPIRES_IN_HOURS")
            .ok()
            .and_then(|s| s.parse().ok())
            .unwrap_or(1); // Default to 1 hour

        Ok(Config {
            database_url,
            jwt_secret,
            jwt_expires_in_hours,
            password_salt,
        })
    }
}
