use crate::config::Config;
use crate::errors::{AppError, Result};
use crate::models::Claims;
use argon2::{
    password_hash::{PasswordHash, PasswordHasher, PasswordVerifier, SaltString},
    Argon2,
};
use chrono::{Duration, Utc};
use jsonwebtoken::{decode, encode, DecodingKey, EncodingKey, Header, Validation};
use hex;

// --- Password Hashing ---

pub fn hash_password(password: &str, config: &Config) -> Result<String> {
    // Decode the hex salt from config
    let salt_bytes = hex::decode(&config.password_salt)
        .map_err(|e| AppError::InternalServerError(format!("Failed to decode salt: {}", e)))?;
    let salt = SaltString::encode_b64(&salt_bytes)
         .map_err(|e| AppError::InternalServerError(format!("Failed to encode salt to b64: {}", e)))?;


    // Argon2 with default params (recommended)
    let argon2 = Argon2::default();

    // Hash password to PHC string format
    let password_hash = argon2
        .hash_password(password.as_bytes(), &salt)
        .map_err(AppError::PasswordHashParseError)?
        .to_string();
    Ok(password_hash)
}

pub fn verify_password(password_hash_str: &str, password: &str) -> Result<bool> {
    let parsed_hash = PasswordHash::new(password_hash_str)
        .map_err(AppError::PasswordHashParseError)?;

    Ok(Argon2::default()
        .verify_password(password.as_bytes(), &parsed_hash)
        .is_ok())
}

// Basic password strength check (similar to C version)
pub fn is_password_strong(password: &str) -> bool {
    let len = password.len();
    if len < 8 { return false; }

    let mut has_upper = false;
    let mut has_lower = false;
    let mut has_digit = false;
    let mut has_special = false; // Consider defining what counts as special

    for c in password.chars() {
        if c.is_uppercase() { has_upper = true; }
        else if c.is_lowercase() { has_lower = true; }
        else if c.is_digit(10) { has_digit = true; }
        // Basic check for non-alphanumeric - adjust as needed
        else if !c.is_alphanumeric() { has_special = true; }
    }

    let criteria_met = [has_upper, has_lower, has_digit, has_special].iter().filter(|&&x| x).count();
    criteria_met >= 3
}


// --- JWT ---

pub fn create_jwt(user_id: i64, username: &str, config: &Config) -> Result<String> {
    let now = Utc::now();
    // Use chrono::Duration directly
    let expiration = now + Duration::hours(config.jwt_expires_in_hours as i64);

    let claims = Claims {
        sub: username.to_owned(),
        user_id,
        exp: expiration.timestamp() as usize,
        iat: now.timestamp() as usize,
    };

    let header = Header::default(); // Default is HS256
    let encoding_key = EncodingKey::from_secret(config.jwt_secret.as_ref());

    encode(&header, &claims, &encoding_key).map_err(AppError::JwtError)
}

// Example function to decode/validate JWT (could be used in middleware)
pub fn validate_jwt(token: &str, config: &Config) -> Result<Claims> {
    let decoding_key = DecodingKey::from_secret(config.jwt_secret.as_ref());
    // Specify the algorithm used for encoding
    let mut validation = Validation::new(jsonwebtoken::Algorithm::HS256);
    // Add other validation rules if needed (e.g., audience, issuer)
    validation.validate_exp = true; // Ensure 'exp' claim is checked

    decode::<Claims>(token, &decoding_key, &validation)
        .map(|data| data.claims)
        .map_err(AppError::JwtError)
}
