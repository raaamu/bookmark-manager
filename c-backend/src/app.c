#include "../include/app.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <jansson.h>

// Forward declarations
char* hash_password(const char* password);

// Base64url-encode input buffer (used for JWK key construction)
static char* base64url_encode(const unsigned char* input, size_t len) {
    size_t b64_len = 4 * ((len + 2) / 3) + 1;
    char* b64 = malloc(b64_len);
    if (!b64) return NULL;

    int encoded_len = EVP_EncodeBlock((unsigned char*)b64, input, (int)len);

    char* result = malloc(encoded_len + 1);
    if (!result) { free(b64); return NULL; }

    int j = 0;
    for (int i = 0; i < encoded_len; i++) {
        if (b64[i] == '+')      result[j++] = '-';
        else if (b64[i] == '/') result[j++] = '_';
        else if (b64[i] != '=') result[j++] = b64[i];
    }
    result[j] = '\0';
    free(b64);
    return result;
}

// Static variables
static sqlite3* db = NULL;
static struct MHD_Daemon* http_daemon = NULL;
static char* jwt_secret = NULL; // Will be initialized with a secure random key

// Error messages
static const char* error_messages[] = {
    "Success",
    "Database initialization failed",
    "Database query failed",
    "User already exists",
    "User not found",
    "Invalid credentials",
    "Bookmark already exists",
    "Bookmark not found",
    "Invalid input",
    "Server error"
};

const char* get_error_message(ErrorCode code) {
    if (code >= 0 && code < sizeof(error_messages) / sizeof(error_messages[0])) {
        return error_messages[code];
    }
    return "Unknown error";
}

HttpResponse* create_error_response(ErrorCode code) {
    HttpResponse* response = malloc(sizeof(HttpResponse));
    if (!response) return NULL;

    json_t* error_obj = json_object();
    if (!error_obj) {
        free(response);
        return NULL;
    }

    if (json_object_set_new(error_obj, "error", json_string(get_error_message(code))) != 0) {
        json_decref(error_obj);
        free(response);
        return NULL;
    }

    char* json_str = json_dumps(error_obj, JSON_COMPACT);
    json_decref(error_obj);
    
    if (!json_str) {
        free(response);
        return NULL;
    }

    response->status_code = (code == SUCCESS) ? 200 : 400;
    response->body = json_str;
    response->body_length = strlen(json_str);

    return response;
}

void free_response(HttpResponse* response) {
    if (response) {
        free(response->body);
        free(response);
    }
}

// Database functions
ErrorCode init_database() {
    int rc;
    int retry_count = 0;
    const int max_retries = 3;
    const int retry_delay_ms = 500;
    
    // Try to open the database with retries
    do {
        rc = sqlite3_open_v2("bookmarks.db", &db, 
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, 
                           NULL);
        
        if (rc != SQLITE_OK) {
            if (++retry_count >= max_retries) {
                return ERROR_DB_INIT;
            }
            
            // Wait before retrying
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = retry_delay_ms * 1000000;
            nanosleep(&ts, NULL);
            
            // Close the failed connection attempt
            if (db) {
                sqlite3_close(db);
                db = NULL;
            }
        }
    } while (rc != SQLITE_OK);
    
    // Set busy timeout to prevent "database is locked" errors
    sqlite3_busy_timeout(db, 5000);  // 5 seconds

    // Generate a secure random JWT secret if not already initialized
    if (!jwt_secret) {
        unsigned char random_bytes[32];
        if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
            sqlite3_close(db);
            db = NULL;
            return ERROR_SERVER;
        }
        
        jwt_secret = malloc(2 * sizeof(random_bytes) + 1);
        if (!jwt_secret) {
            sqlite3_close(db);
            db = NULL;
            return ERROR_SERVER;
        }
        
        for (size_t i = 0; i < sizeof(random_bytes); i++) {
            snprintf(jwt_secret + (i * 2), 3, "%02x", random_bytes[i]);
        }
        jwt_secret[2 * sizeof(random_bytes)] = '\0';
    }

    const char* create_users_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "failed_attempts INTEGER DEFAULT 0,"
        "last_attempt TIMESTAMP"
        ");";

    const char* create_bookmarks_table =
        "CREATE TABLE IF NOT EXISTS bookmarks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "title TEXT NOT NULL,"
        "url TEXT NOT NULL,"
        "description TEXT,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (user_id) REFERENCES users(id)"
        ");";

    char* err_msg = NULL;
    rc = sqlite3_exec(db, create_users_table, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;
        return ERROR_DB_INIT;
    }

    rc = sqlite3_exec(db, create_bookmarks_table, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;
        return ERROR_DB_INIT;
    }

    return SUCCESS;
}

ErrorCode create_user(const char* username, const char* password) {
    if (!username || !password) return ERROR_INVALID_INPUT;
    if (!db) return ERROR_DB_INIT;
    
    // Validate password strength
    if (!is_password_strong(password)) return ERROR_INVALID_INPUT;

    char* hashed_password = hash_password(password);
    if (!hashed_password) return ERROR_SERVER;

    const char* sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(hashed_password);
        return ERROR_DB_QUERY;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_CONSTRAINT) {
        sqlite3_finalize(stmt);
        free(hashed_password);
        return ERROR_USER_EXISTS;
    }
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        free(hashed_password);
        return ERROR_DB_QUERY;
    }

    sqlite3_finalize(stmt);
    free(hashed_password);

    return SUCCESS;
}

ErrorCode authenticate_user(const char* username, const char* password, char** token) {
    if (!username || !password || !token) return ERROR_INVALID_INPUT;
    if (!db) return ERROR_DB_INIT;
    *token = NULL;  // Initialize to NULL for safety

    // Check for brute force attempts
    static time_t last_failed_attempt = 0;
    static int failed_attempts = 0;
    static char last_username[256] = {0};
    
    time_t now = time(NULL);
    
    // If it's the same username and within 5 minutes of last failure
    if (strcmp(last_username, username) == 0 && now - last_failed_attempt < 300) {
        // If too many failed attempts, enforce delay
        if (failed_attempts >= 5) {
            // Calculate remaining lockout time
            int lockout_time = 300 - (now - last_failed_attempt);
            if (lockout_time > 0) {
                return ERROR_INVALID_CREDENTIALS;
            }
            // Reset counter after lockout period
            failed_attempts = 0;
        }
    } else if (strcmp(last_username, username) != 0) {
        // Different username, reset counter
        failed_attempts = 0;
        strncpy(last_username, username, sizeof(last_username) - 1);
        last_username[sizeof(last_username) - 1] = '\0';
    }

    char* hashed_password = hash_password(password);
    if (!hashed_password) return ERROR_SERVER;

    const char* sql = "SELECT id FROM users WHERE username = ? AND password = ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(hashed_password);
        return ERROR_DB_QUERY;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    free(hashed_password);

    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        // Record failed attempt
        last_failed_attempt = time(NULL);
        failed_attempts++;
        return ERROR_INVALID_CREDENTIALS;
    }

    // Reset failed attempts on successful login
    failed_attempts = 0;

    int user_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    // Create JWT token using libjwt 3.x builder API
    // Build a JWK JSON object for the HMAC-SHA256 secret key
    char* b64url_key = base64url_encode((const unsigned char*)jwt_secret, strlen(jwt_secret));
    if (!b64url_key) return ERROR_SERVER;

    size_t jwk_json_len = strlen(b64url_key) + 64;
    char* jwk_json = malloc(jwk_json_len);
    if (!jwk_json) { free(b64url_key); return ERROR_SERVER; }
    snprintf(jwk_json, jwk_json_len,
             "{\"kty\":\"oct\",\"alg\":\"HS256\",\"k\":\"%s\"}", b64url_key);
    free(b64url_key);

    jwk_set_t *jwk_set = jwks_create(jwk_json);
    free(jwk_json);
    if (!jwk_set || jwks_error_any(jwk_set)) {
        if (jwk_set) jwks_free(jwk_set);
        return ERROR_SERVER;
    }

    const jwk_item_t *key_item = jwks_item_get(jwk_set, 0);
    if (!key_item) { jwks_free(jwk_set); return ERROR_SERVER; }

    jwt_builder_t *builder = jwt_builder_new();
    if (!builder) { jwks_free(jwk_set); return ERROR_SERVER; }

    int ret = jwt_builder_setkey(builder, JWT_ALG_HS256, key_item);
    jwks_free(jwk_set);
    if (ret != 0) { jwt_builder_free(builder); return ERROR_SERVER; }

    // Set claims (iat is added automatically by the builder)
    time_t now_time = time(NULL);
    jwt_value_t jval;

    jwt_set_SET_INT(&jval, "exp", (long)(now_time + 3600));
    if (jwt_builder_claim_set(builder, &jval) != JWT_VALUE_ERR_NONE) {
        jwt_builder_free(builder); return ERROR_SERVER;
    }

    jwt_set_SET_STR(&jval, "sub", username);
    if (jwt_builder_claim_set(builder, &jval) != JWT_VALUE_ERR_NONE) {
        jwt_builder_free(builder); return ERROR_SERVER;
    }

    char user_id_str[16];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    jwt_set_SET_STR(&jval, "user_id", user_id_str);
    if (jwt_builder_claim_set(builder, &jval) != JWT_VALUE_ERR_NONE) {
        jwt_builder_free(builder); return ERROR_SERVER;
    }

    *token = jwt_builder_generate(builder);
    jwt_builder_free(builder);

    if (!*token) return ERROR_SERVER;

    return SUCCESS;
}

ErrorCode add_bookmark(const char* username, const char* title, const char* url, const char* description) {
    if (!username || !title || !url) return ERROR_INVALID_INPUT;
    if (!db) return ERROR_DB_INIT;
    
    // Validate URL format
    if (!is_valid_url(url)) return ERROR_INVALID_INPUT;
    
    // Use empty string if description is NULL
    const char* safe_description = description ? description : "";

    // Check if bookmark exists
    if (bookmark_exists(username, url)) return ERROR_BOOKMARK_EXISTS;

    // Get user ID
    const char* get_user_sql = "SELECT id FROM users WHERE username = ?";
    sqlite3_stmt* user_stmt;
    int rc = sqlite3_prepare_v2(db, get_user_sql, -1, &user_stmt, NULL);
    if (rc != SQLITE_OK) return ERROR_DB_QUERY;

    sqlite3_bind_text(user_stmt, 1, username, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(user_stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(user_stmt);
        return ERROR_USER_NOT_FOUND;
    }

    int user_id = sqlite3_column_int(user_stmt, 0);
    sqlite3_finalize(user_stmt);

    // Insert bookmark
    const char* insert_sql = "INSERT INTO bookmarks (user_id, title, url, description) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* insert_stmt;
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, NULL);
    if (rc != SQLITE_OK) return ERROR_DB_QUERY;

    sqlite3_bind_int(insert_stmt, 1, user_id);
    sqlite3_bind_text(insert_stmt, 2, title, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_stmt, 3, url, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_stmt, 4, safe_description, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(insert_stmt);
    sqlite3_finalize(insert_stmt);

    if (rc != SQLITE_DONE) return ERROR_DB_QUERY;
    return SUCCESS;
}

bool bookmark_exists(const char* username, const char* url) {
    if (!username || !url || !db) return false;

    const char* sql = "SELECT 1 FROM bookmarks b JOIN users u ON b.user_id = u.id WHERE u.username = ? AND b.url = ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return false;
    }
    rc = sqlite3_bind_text(stmt, 2, url, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return exists;
}

bool is_valid_url(const char* url) {
    if (!url) return false;
    
    // Basic URL validation - must start with http:// or https://
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        return false;
    }
    
    // Check for minimum length and basic structure
    size_t len = strlen(url);
    if (len < 10) return false; // http://a.b is minimum valid URL
    
    // Check for at least one dot after the protocol
    const char* domain_start = strstr(url, "://");
    if (!domain_start) return false;
    
    domain_start += 3; // Skip past ://
    const char* dot = strchr(domain_start, '.');
    if (!dot || dot == domain_start) return false;
    
    return true;
}

bool is_password_strong(const char* password) {
    if (!password) return false;
    
    size_t len = strlen(password);
    if (len < 8) return false; // Minimum 8 characters
    
    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_special = false;
    
    for (size_t i = 0; i < len; i++) {
        char c = password[i];
        if (isupper(c)) has_upper = true;
        else if (islower(c)) has_lower = true;
        else if (isdigit(c)) has_digit = true;
        else has_special = true;
    }
    
    // Require at least 3 of the 4 character types
    int criteria_met = 0;
    if (has_upper) criteria_met++;
    if (has_lower) criteria_met++;
    if (has_digit) criteria_met++;
    if (has_special) criteria_met++;
    
    return criteria_met >= 3;
}

ErrorCode get_bookmarks(const char* username, char** json_response) {
    if (!username || !json_response) return ERROR_INVALID_INPUT;
    if (!db) return ERROR_DB_INIT;
    *json_response = NULL;  // Initialize to NULL for safety

    const char* sql = "SELECT b.id, b.title, b.url, b.description, b.created_at, b.updated_at "
                     "FROM bookmarks b JOIN users u ON b.user_id = u.id WHERE u.username = ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return ERROR_DB_QUERY;

    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return ERROR_DB_QUERY;
    }

    json_t* bookmarks_array = json_array();
    if (!bookmarks_array) {
        sqlite3_finalize(stmt);
        return ERROR_SERVER;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        json_t* bookmark = json_object();
        if (!bookmark) {
            json_decref(bookmarks_array);
            sqlite3_finalize(stmt);
            return ERROR_SERVER;
        }

        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* url = (const char*)sqlite3_column_text(stmt, 2);
        const char* description = (const char*)sqlite3_column_text(stmt, 3);
        const char* created_at = (const char*)sqlite3_column_text(stmt, 4);
        const char* updated_at = (const char*)sqlite3_column_text(stmt, 5);

        json_object_set_new(bookmark, "id", json_integer(sqlite3_column_int(stmt, 0)));
        json_object_set_new(bookmark, "title", json_string(title ? title : ""));
        json_object_set_new(bookmark, "url", json_string(url ? url : ""));
        json_object_set_new(bookmark, "description", json_string(description ? description : ""));
        json_object_set_new(bookmark, "created_at", json_string(created_at ? created_at : ""));
        json_object_set_new(bookmark, "updated_at", json_string(updated_at ? updated_at : ""));
        
        json_array_append_new(bookmarks_array, bookmark);
    }
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        json_decref(bookmarks_array);
        return ERROR_DB_QUERY;
    }

    *json_response = json_dumps(bookmarks_array, JSON_COMPACT);
    json_decref(bookmarks_array);

    if (!*json_response) {
        return ERROR_SERVER;
    }

    return SUCCESS;
}

ErrorCode search_bookmarks(const char* username, const char* query, char** json_response) {
    if (!username || !query || !json_response) return ERROR_INVALID_INPUT;
    if (!db) return ERROR_DB_INIT;
    *json_response = NULL;  // Initialize to NULL for safety

    // Convert query to lowercase for case-insensitive search
    char* query_lower = strdup(query);
    if (!query_lower) return ERROR_SERVER;
    
    for (char* p = query_lower; *p; ++p) {
        *p = tolower(*p);
    }

    const char* sql = "SELECT b.id, b.title, b.url, b.description, b.created_at, b.updated_at "
                     "FROM bookmarks b JOIN users u ON b.user_id = u.id WHERE u.username = ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(query_lower);
        return ERROR_DB_QUERY;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    json_t* bookmarks_array = json_array();
    if (!bookmarks_array) {
        free(query_lower);
        sqlite3_finalize(stmt);
        return ERROR_SERVER;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Get values from the database
        int id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* url = (const char*)sqlite3_column_text(stmt, 2);
        const char* description = (const char*)sqlite3_column_text(stmt, 3);
        const char* created_at = (const char*)sqlite3_column_text(stmt, 4);
        const char* updated_at = (const char*)sqlite3_column_text(stmt, 5);
        
        // Convert strings to lowercase for case-insensitive comparison
        char* title_lower = title ? strdup(title) : NULL;
        char* url_lower = url ? strdup(url) : NULL;
        char* description_lower = description ? strdup(description) : NULL;
        
        if (title_lower) {
            for (char* p = title_lower; *p; ++p) *p = tolower(*p);
        }
        
        if (url_lower) {
            for (char* p = url_lower; *p; ++p) *p = tolower(*p);
        }
        
        if (description_lower) {
            for (char* p = description_lower; *p; ++p) *p = tolower(*p);
        }
        
        // Check if any field contains the search query
        bool matches = false;
        if ((title_lower && strstr(title_lower, query_lower)) ||
            (url_lower && strstr(url_lower, query_lower)) ||
            (description_lower && strstr(description_lower, query_lower))) {
            matches = true;
        }
        
        // Free temporary strings
        free(title_lower);
        free(url_lower);
        free(description_lower);
        
        // If there's a match, add to results
        if (matches) {
            json_t* bookmark = json_object();
            if (!bookmark) {
                json_decref(bookmarks_array);
                free(query_lower);
                sqlite3_finalize(stmt);
                return ERROR_SERVER;
            }
            
            json_object_set_new(bookmark, "id", json_integer(id));
            json_object_set_new(bookmark, "title", json_string(title ? title : ""));
            json_object_set_new(bookmark, "url", json_string(url ? url : ""));
            json_object_set_new(bookmark, "description", json_string(description ? description : ""));
            json_object_set_new(bookmark, "created_at", json_string(created_at ? created_at : ""));
            json_object_set_new(bookmark, "updated_at", json_string(updated_at ? updated_at : ""));
            json_array_append_new(bookmarks_array, bookmark);
        }
    }
    
    sqlite3_finalize(stmt);
    free(query_lower);

    if (rc != SQLITE_DONE) {
        json_decref(bookmarks_array);
        return ERROR_DB_QUERY;
    }

    *json_response = json_dumps(bookmarks_array, JSON_COMPACT);
    json_decref(bookmarks_array);

    if (!*json_response) {
        return ERROR_SERVER;
    }

    return SUCCESS;
}

ErrorCode delete_bookmark(const char* username, int bookmark_id) {
    if (!username || bookmark_id <= 0) return ERROR_INVALID_INPUT;
    if (!db) return ERROR_DB_INIT;

    const char* sql = "DELETE FROM bookmarks WHERE id = ? AND user_id = (SELECT id FROM users WHERE username = ?)";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return ERROR_DB_QUERY;

    sqlite3_bind_int(stmt, 1, bookmark_id);
    sqlite3_bind_text(stmt, 2, username, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return ERROR_DB_QUERY;
    if (sqlite3_changes(db) == 0) return ERROR_BOOKMARK_NOT_FOUND;

    return SUCCESS;
}

char* hash_password(const char* password) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    // Create a message digest context
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) return NULL;
    
    // Initialize the digest operation
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        return NULL;
    }
    
    // Provide the message to be hashed
    if (EVP_DigestUpdate(mdctx, password, strlen(password)) != 1) {
        EVP_MD_CTX_free(mdctx);
        return NULL;
    }
    
    // Finalize the digest operation
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return NULL;
    }
    
    // Clean up the context
    EVP_MD_CTX_free(mdctx);
    
    // Convert the binary hash to a hex string
    char* hashed = malloc(2 * hash_len + 1);
    if (!hashed) return NULL;
    
    for (unsigned int i = 0; i < hash_len; i++) {
        snprintf(hashed + (i * 2), 3, "%02x", hash[i]);
    }
    hashed[2 * hash_len] = '\0';
    
    return hashed;
}

enum MHD_Result handle_request(void* cls, struct MHD_Connection* connection,
                             const char* url, const char* method,
                             const char* version, const char* upload_data,
                             size_t* upload_data_size, void** con_cls) {
    // TODO: Implement request handling
    return MHD_NO;
}

// Cleanup function to free resources
void cleanup_server() {
    if (http_daemon) {
        MHD_stop_daemon(http_daemon);
        http_daemon = NULL;
    }
    
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
    
    if (jwt_secret) {
        free(jwt_secret);
        jwt_secret = NULL;
    }
}

ErrorCode start_server(int port) {
    ErrorCode rc = init_database();
    if (rc != SUCCESS) return rc;

    http_daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL,
                                  &handle_request, NULL, MHD_OPTION_END);
    if (!http_daemon) {
        cleanup_server();
        return ERROR_SERVER;
    }

    return SUCCESS;
}
