#include "app.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <cctype>

BookmarkManager::BookmarkManager() : db(nullptr) {
    // Generate a random JWT secret
    unsigned char secret[32];
    RAND_bytes(secret, sizeof(secret));
    std::stringstream ss;
    for (int i = 0; i < sizeof(secret); i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)secret[i];
    }
    jwt_secret = ss.str();

    if (!initDatabase()) {
        throw std::runtime_error("Failed to initialize database");
    }
}

BookmarkManager::~BookmarkManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool BookmarkManager::initDatabase() {
    int rc = sqlite3_open("bookmarks.db", &db);
    if (rc) {
        return false;
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS bookmarks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            title TEXT NOT NULL,
            url TEXT NOT NULL,
            description TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id)
        );
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

std::string BookmarkManager::hashPassword(const std::string& password) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, password.c_str(), password.length());
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;
    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hashLen; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool BookmarkManager::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

bool BookmarkManager::bookmarkExists(const std::string& username, const std::string& url) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(*) FROM bookmarks WHERE user_id = (SELECT id FROM users WHERE username = ?) AND url = ?";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_STATIC);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
}

bool BookmarkManager::createUser(const std::string& username, const std::string& password) {
    std::string hashedPassword = hashPassword(password);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool BookmarkManager::verifyUser(const std::string& username, const std::string& password) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT password FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        success = verifyPassword(password, storedHash);
    }

    sqlite3_finalize(stmt);
    return success;
}

std::string BookmarkManager::generateToken(const std::string& username) {
    auto now = std::chrono::system_clock::now();
    auto token = jwt::create()
        .set_issuer("bookmark-manager")
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::hours{24})
        .set_payload_claim("username", jwt::claim(username))
        .sign(jwt::algorithm::hs256{jwt_secret});
    
    return token;
}

bool BookmarkManager::verifyToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{jwt_secret})
            .with_issuer("bookmark-manager");
        
        verifier.verify(decoded);
        return true;
    } catch (...) {
        return false;
    }
}

bool BookmarkManager::addBookmark(const std::string& username, const std::string& title, const std::string& url, const std::string& description) {
    // Check if bookmark already exists
    if (bookmarkExists(username, url)) {
        return false;
    }

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO bookmarks (user_id, title, url, description) VALUES ((SELECT id FROM users WHERE username = ?), ?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, description.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::vector<crow::json::wvalue> BookmarkManager::getBookmarks(const std::string& username) {
    std::vector<crow::json::wvalue> bookmarks;
    
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, title, url, description, created_at, updated_at FROM bookmarks WHERE user_id = (SELECT id FROM users WHERE username = ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return bookmarks;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        crow::json::wvalue bookmark;
        bookmark["id"] = sqlite3_column_int(stmt, 0);
        bookmark["title"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        bookmark["url"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        
        // Handle description which might be NULL
        if (sqlite3_column_text(stmt, 3)) {
            bookmark["description"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        } else {
            bookmark["description"] = "";
        }
        
        // Handle timestamps
        if (sqlite3_column_text(stmt, 4)) {
            bookmark["created_at"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        }
        
        if (sqlite3_column_text(stmt, 5)) {
            bookmark["updated_at"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        }
        
        bookmarks.push_back(std::move(bookmark));
    }

    sqlite3_finalize(stmt);
    return bookmarks;
}

std::vector<crow::json::wvalue> BookmarkManager::searchBookmarks(const std::string& username, const std::string& query) {
    std::vector<crow::json::wvalue> bookmarks;
    
    // Get all bookmarks for the user
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, title, url, description, created_at, updated_at FROM bookmarks WHERE user_id = (SELECT id FROM users WHERE username = ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return bookmarks;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    // Convert query to lowercase for case-insensitive search
    std::string queryLower = query;
    std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Get bookmark data
        int id = sqlite3_column_int(stmt, 0);
        std::string title = sqlite3_column_text(stmt, 1) ? 
            std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) : "";
        std::string url = sqlite3_column_text(stmt, 2) ? 
            std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))) : "";
        std::string description = sqlite3_column_text(stmt, 3) ? 
            std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))) : "";
        std::string created_at = sqlite3_column_text(stmt, 4) ? 
            std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))) : "";
        std::string updated_at = sqlite3_column_text(stmt, 5) ? 
            std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))) : "";
        
        // Convert to lowercase for case-insensitive search
        std::string titleLower = title;
        std::string urlLower = url;
        std::string descriptionLower = description;
        
        std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        std::transform(descriptionLower.begin(), descriptionLower.end(), descriptionLower.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        
        // Check if any field contains the search query
        if (titleLower.find(queryLower) != std::string::npos ||
            urlLower.find(queryLower) != std::string::npos ||
            descriptionLower.find(queryLower) != std::string::npos) {
            
            crow::json::wvalue bookmark;
            bookmark["id"] = id;
            bookmark["title"] = title;
            bookmark["url"] = url;
            bookmark["description"] = description;
            bookmark["created_at"] = created_at;
            bookmark["updated_at"] = updated_at;
            
            bookmarks.push_back(std::move(bookmark));
        }
    }

    sqlite3_finalize(stmt);
    return bookmarks;
}

bool BookmarkManager::deleteBookmark(const std::string& username, int id) {
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM bookmarks WHERE id = ? AND user_id = (SELECT id FROM users WHERE username = ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

void BookmarkManager::run(int port) {
    crow::App<crow::CORSHandler> app;

    // Enable CORS
    app.get_middleware<crow::CORSHandler>()
        .global()
        .headers("*")
        .methods("POST"_method, "GET"_method, "DELETE"_method);

    // Register endpoint
    CROW_ROUTE(app, "/register")
        .methods("POST"_method)
    ([this](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json || !json.has("username") || !json.has("password")) {
            return crow::response(400, "Invalid request");
        }

        std::string username = json["username"].s();
        std::string password = json["password"].s();

        if (createUser(username, password)) {
            return crow::response(201, "User created successfully");
        } else {
            return crow::response(400, "Username already exists");
        }
    });

    // Login endpoint
    CROW_ROUTE(app, "/login")
        .methods("POST"_method)
    ([this](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json || !json.has("username") || !json.has("password")) {
            return crow::response(400, "Invalid request");
        }

        std::string username = json["username"].s();
        std::string password = json["password"].s();

        if (verifyUser(username, password)) {
            std::string token = generateToken(username);
            crow::json::wvalue response;
            response["token"] = token;
            return crow::response(200, response);
        } else {
            return crow::response(401, "Invalid credentials");
        }
    });

    // Bookmark endpoints
    CROW_ROUTE(app, "/bookmarks")
        .methods("GET"_method)
    ([this](const crow::request& req) {
        auto auth = req.get_header_value("Authorization");
        if (auth.empty() || !verifyToken(auth.substr(7))) { // Remove "Bearer " prefix
            return crow::response(401, "Unauthorized");
        }

        auto decoded = jwt::decode(auth.substr(7));
        std::string username = decoded.get_payload_claim("username").as_string();
        
        // Check if search query parameter exists
        auto query = req.url_params.get("q");
        std::vector<crow::json::wvalue> bookmarks;
        
        if (query && strlen(query) > 0) {
            // Search bookmarks
            bookmarks = searchBookmarks(username, query);
        } else {
            // Get all bookmarks
            bookmarks = getBookmarks(username);
        }
        
        crow::json::wvalue result(bookmarks);
        return crow::response(200, result);
    });

    CROW_ROUTE(app, "/bookmarks")
        .methods("POST"_method)
    ([this](const crow::request& req) {
        auto auth = req.get_header_value("Authorization");
        if (auth.empty() || !verifyToken(auth.substr(7))) {
            return crow::response(401, "Unauthorized");
        }

        auto json = crow::json::load(req.body);
        if (!json || !json.has("title") || !json.has("url")) {
            return crow::response(400, "Invalid request");
        }

        auto decoded = jwt::decode(auth.substr(7));
        std::string username = decoded.get_payload_claim("username").as_string();
        
        std::string description = "";
        if (json.has("description")) {
            description = json["description"].s();
        }

        if (addBookmark(username, json["title"].s(), json["url"].s(), description)) {
            return crow::response(201, "Bookmark added successfully");
        } else {
            return crow::response(409, "Bookmark with this URL already exists");
        }
    });

    CROW_ROUTE(app, "/bookmarks/<int>")
        .methods("DELETE"_method)
    ([this](const crow::request& req, int id) {
        auto auth = req.get_header_value("Authorization");
        if (auth.empty() || !verifyToken(auth.substr(7))) {
            return crow::response(401, "Unauthorized");
        }

        auto decoded = jwt::decode(auth.substr(7));
        std::string username = decoded.get_payload_claim("username").as_string();

        if (deleteBookmark(username, id)) {
            return crow::response(200, "Bookmark deleted successfully");
        } else {
            return crow::response(404, "Bookmark not found");
        }
    });

    app.port(port).multithreaded().run();
} 