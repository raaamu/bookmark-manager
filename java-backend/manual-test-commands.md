# Manual API Testing Commands

This document provides curl commands to manually test the Java backend API.

## Prerequisites

- The Java backend should be running on port 42000
- You can use the `run-without-docker.sh` script to start the backend

## Testing Steps

### 1. Register a new user

```bash
curl -X POST "http://localhost:42000/register" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"password123"}'
```

Expected response:
```json
{"message": "User registered successfully"}
```

### 2. Login with the user

```bash
curl -X POST "http://localhost:42000/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"password123"}'
```

Expected response:
```json
{
  "token": "eyJhbGciOiJIUzI1NiJ9...",
  "username": "testuser",
  "expiresIn": 3600000
}
```

Save the token for subsequent requests.

### 3. Get bookmarks (initially empty)

```bash
curl -X GET "http://localhost:42000/bookmarks" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

Expected response:
```json
[]
```

### 4. Create a bookmark

```bash
curl -X POST "http://localhost:42000/bookmarks" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE" \
  -d '{"title":"Test Bookmark","url":"https://example.com","description":"This is a test bookmark"}'
```

Expected response:
```json
{
  "id": 1,
  "title": "Test Bookmark",
  "url": "https://example.com",
  "description": "This is a test bookmark"
}
```

### 5. Get bookmarks (should contain the created bookmark)

```bash
curl -X GET "http://localhost:42000/bookmarks" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

Expected response:
```json
[
  {
    "id": 1,
    "title": "Test Bookmark",
    "url": "https://example.com",
    "description": "This is a test bookmark"
  }
]
```

### 6. Search bookmarks

```bash
curl -X GET "http://localhost:42000/bookmarks?q=test" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

Expected response:
```json
[
  {
    "id": 1,
    "title": "Test Bookmark",
    "url": "https://example.com",
    "description": "This is a test bookmark"
  }
]
```

### 7. Delete a bookmark

```bash
curl -X DELETE "http://localhost:42000/bookmarks/1" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

Expected response:
```json
{"message": "Bookmark deleted successfully"}
```

### 8. Get bookmarks after deletion (should be empty)

```bash
curl -X GET "http://localhost:42000/bookmarks" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

Expected response:
```json
[]
```
