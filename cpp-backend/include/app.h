#pragma once

#include "crow/app.h"
#include "crow/middlewares/cors.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <jwt-cpp/jwt.h>

class BookmarkManager {
public:
    BookmarkManager();
    ~BookmarkManager();

    void run(int port = 5000);

private:
    // Database operations
    bool initDatabase();
    bool createUser(const std::string& username, const std::string& password);
    bool verifyUser(const std::string& username, const std::string& password);
    std::string generateToken(const std::string& username);
    bool verifyToken(const std::string& token);
    
    // Bookmark operations
    bool addBookmark(const std::string& username, const std::string& title, const std::string& url, const std::string& description = "");
    std::vector<crow::json::wvalue> getBookmarks(const std::string& username);
    std::vector<crow::json::wvalue> searchBookmarks(const std::string& username, const std::string& query);
    bool deleteBookmark(const std::string& username, int id);

    // Helper functions
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    bool bookmarkExists(const std::string& username, const std::string& url);

    sqlite3* db;
    std::string jwt_secret;
}; 