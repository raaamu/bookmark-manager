# .NET Core Backend for Bookmark Manager

This is a .NET Core implementation of the Bookmark Manager backend API.

## Technologies Used

- .NET 8.0
- ASP.NET Core Web API
- Entity Framework Core with SQLite
- JWT Authentication
- Swagger/OpenAPI

## API Endpoints

- `POST /register`: Register a new user
- `POST /login`: Authenticate and receive JWT token
- `GET /bookmarks`: Get all bookmarks for authenticated user
- `GET /bookmarks?q=search_term`: Search bookmarks by title, URL, or description
- `POST /bookmarks`: Create a new bookmark
- `DELETE /bookmarks/{id}`: Delete a bookmark
- `GET /health`: Health check endpoint

## Getting Started

### Prerequisites

- .NET 8.0 SDK or later

### Running the Application

1. Navigate to the project directory:
   ```bash
   cd dotnet-backend
   ```

2. Restore dependencies:
   ```bash
   dotnet restore
   ```

3. Run the application:
   ```bash
   dotnet run
   ```

The API will be available at `http://localhost:42000`.

### Using the API

1. Register a new user:
   ```bash
   curl -X POST http://localhost:42000/register -H "Content-Type: application/json" -d '{"username":"testuser","password":"password123"}'
   ```

2. Login to get a JWT token:
   ```bash
   curl -X POST http://localhost:42000/login -H "Content-Type: application/json" -d '{"username":"testuser","password":"password123"}'
   ```

3. Use the JWT token to access protected endpoints:
   ```bash
   curl -X GET http://localhost:42000/bookmarks -H "Authorization: Bearer YOUR_JWT_TOKEN"
   ```

## Security Features

- Password hashing using BCrypt
- JWT token authentication
- Authorization policies
- CORS configuration for frontend access
- Input validation

## Development

The application uses Entity Framework Core with SQLite for data persistence. The database file will be created automatically in the project directory when the application runs for the first time.
