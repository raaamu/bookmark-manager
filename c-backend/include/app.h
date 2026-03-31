#ifndef APP_H
#define APP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <microhttpd.h>
#include <jansson.h>
#include <jwt.h>
#include <openssl/evp.h>

// Error codes
typedef enum {
    SUCCESS = 0,
    ERROR_DB_INIT = 1,
    ERROR_DB_QUERY = 2,
    ERROR_USER_EXISTS = 3,
    ERROR_USER_NOT_FOUND = 4,
    ERROR_INVALID_CREDENTIALS = 5,
    ERROR_BOOKMARK_EXISTS = 6,
    ERROR_BOOKMARK_NOT_FOUND = 7,
    ERROR_INVALID_INPUT = 8,
    ERROR_SERVER = 9
} ErrorCode;

// HTTP response structure
typedef struct {
    int status_code;
    char* body;
    size_t body_length;
} HttpResponse;

// Error handling
const char* get_error_message(ErrorCode code);
HttpResponse* create_error_response(ErrorCode code);
void free_response(HttpResponse* response);

// Database functions
ErrorCode init_database();
ErrorCode create_user(const char* username, const char* password);
ErrorCode authenticate_user(const char* username, const char* password, char** token);

// Bookmark functions
ErrorCode add_bookmark(const char* username, const char* title, const char* url, const char* description);
ErrorCode get_bookmarks(const char* username, char** json_response);
ErrorCode search_bookmarks(const char* username, const char* query, char** json_response);
ErrorCode delete_bookmark(const char* username, int bookmark_id);
bool bookmark_exists(const char* username, const char* url);

// Server functions
enum MHD_Result handle_request(void* cls, struct MHD_Connection* connection,
                             const char* url, const char* method,
                             const char* version, const char* upload_data,
                             size_t* upload_data_size, void** con_cls);
ErrorCode start_server(int port);

// Utility functions
char* hash_password(const char* password);
bool is_valid_url(const char* url);
bool is_password_strong(const char* password);

#endif // APP_H 