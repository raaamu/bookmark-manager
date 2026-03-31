# Bookmark Manager

A web application for managing bookmarks with multiple backend implementations and an Angular frontend.

## Architecture

The application consists of multiple backend implementations and a frontend:

### Backend Implementations

1. **C Backend**: A RESTful API server built with:
   - libmicrohttpd for HTTP server
   - SQLite for database
   - libjwt for authentication
   - jansson for JSON handling
   - OpenSSL for cryptography

2. **Java Backend**: A Spring Boot application with:
   - Spring Boot 3.x
   - Spring Security with JWT
   - H2 Database
   - Docker support
   - Maven build system

3. **C++ Backend**: A RESTful API server built with:
   - Crow C++ web framework
   - SQLite for database
   - jwt-cpp for authentication
   - CMake build system

4. **Python Backend**: A Flask application with:
   - Flask web framework
   - SQLite for database
   - JWT for authentication
   - Python 3.x

### Angular Frontend
A modern web interface built with:
   - Angular 19.2.5
   - Angular Material
   - RxJS

## Features

- User authentication (register, login, logout)
- JWT-based authentication
- Create, read, and delete bookmarks
- Search bookmarks by title, URL, or description
- Secure password hashing
- Token expiration handling

## API Endpoints

All backends implement the same REST API endpoints:

- `POST /register`: Register a new user
- `POST /login`: Authenticate and receive JWT token
- `GET /bookmarks`: Get all bookmarks for authenticated user
- `GET /bookmarks?q=search_term`: Search bookmarks by title, URL, or description
- `POST /bookmarks`: Create a new bookmark
- `DELETE /bookmarks/:id`: Delete a bookmark

## Building and Running

### C Backend

```bash
cd c-backend
mkdir -p build && cd build
cmake ..
make
./bookmark_manager
```

### Java Backend

```bash
cd java-backend
# Using Maven
./mvnw spring-boot:run

# Using Docker
./run-with-docker.sh
```

### C++ Backend

```bash
cd cpp-backend
mkdir -p build && cd build
cmake ..
make
./bookmark_manager
```

### Python Backend

```bash
cd backend
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install -r requirements.txt
python app.py
```

### Angular Frontend

```bash
cd frontend/bookmark-manager-frontend
npm install
ng serve
```

## Security Notes

- Passwords are securely hashed using appropriate algorithms for each backend
- Authentication is handled with JWT tokens
- CORS is enabled for local development
- Token expiration handling with automatic logout

## Development

This project uses modern programming practices across multiple languages:

- C11 standard for C backend
- C++17 for C++ backend
- Java 17+ for Java backend
- Python 3.x for Python backend
- Angular 19+ for frontend
- RESTful API design
- Separation of concerns
