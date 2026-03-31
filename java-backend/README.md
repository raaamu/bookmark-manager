# Bookmark Manager - Java Backend

This is the Java backend implementation of the Bookmark Manager application. It provides a RESTful API for managing bookmarks with user authentication.

## Features

- User authentication (register, login, logout)
- JWT-based authentication
- Create, read, and delete bookmarks
- Search bookmarks by title, URL, or description
- Token expiration handling

## Tech Stack

- Java 17
- Spring Boot 3.2.0
- Spring Security with JWT
- Spring Data JPA
- H2 Database
- Docker for containerization

## API Endpoints

- `POST /api/auth/register`: Register a new user
- `POST /api/auth/login`: Authenticate and receive JWT token
- `GET /api/bookmarks`: Get all bookmarks for authenticated user
- `GET /api/bookmarks?q=search_term`: Search bookmarks by title, URL, or description
- `POST /api/bookmarks`: Create a new bookmark
- `DELETE /api/bookmarks/{id}`: Delete a bookmark

## Running the Application

### Prerequisites

- Docker and Docker Compose
- Java 17 (for running without Docker)

### Using Docker (Recommended)

#### Development Mode

```bash
./start-dev.sh
```

This will start the application in development mode with:
- Hot reload enabled
- In-memory H2 database
- Debug logging
- All actuator endpoints exposed

#### Production Mode

```bash
./start-prod.sh
```

This will start the application in production mode with:
- Persistent H2 database
- Limited logging
- Limited actuator endpoints

### Without Docker

```bash
./fix-permissions.sh  # Make sure the Maven wrapper is executable
./run-without-docker.sh
```

## Building from Source

```bash
./mvnw clean package
```

## Testing the API

Use the included `test-api.sh` script to test the API endpoints:

```bash
./test-api.sh
```

## Configuration

The application can be configured using the following profiles:

- `dev`: Development configuration with in-memory database
- `prod`: Production configuration with persistent database
- `test`: Test configuration for running tests

## Security Notes

- Authentication is handled with JWT tokens
- Passwords are securely hashed
- Token expiration is set to 1 hour by default
