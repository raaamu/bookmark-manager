# Building the Java Backend

## Prerequisites

To build and run the Java backend, you need:

1. **Java Development Kit (JDK) 17 or later**
   - Download from: https://adoptium.net/ or https://www.oracle.com/java/technologies/downloads/

2. **Maven** (optional, as we use the Maven wrapper)
   - The Maven wrapper (`mvnw`) is included in the project

## Build Instructions

Once Java is installed, follow these steps:

### 1. Set JAVA_HOME environment variable (if not already set)

**For macOS/Linux:**
```bash
export JAVA_HOME=/path/to/your/jdk
```

**For Windows:**
```cmd
set JAVA_HOME=C:\path\to\your\jdk
```

### 2. Build the project

**For macOS/Linux:**
```bash
cd /Users/ramupanayappan/bookmark-manager/java-backend
./mvnw clean package
```

**For Windows:**
```cmd
cd \Users\ramupanayappan\bookmark-manager\java-backend
mvnw.cmd clean package
```

This will:
- Download all required dependencies
- Compile the Java code
- Run tests
- Package the application into a JAR file

### 3. Run the application

**For macOS/Linux:**
```bash
./mvnw spring-boot:run
```

**For Windows:**
```cmd
mvnw.cmd spring-boot:run
```

The application will start and listen on port 42000.

## Alternative: Build with Docker

If you have Docker installed, you can build and run the application without installing Java:

```bash
# Create a Dockerfile
echo 'FROM maven:3.9-eclipse-temurin-17 AS build
WORKDIR /app
COPY . .
RUN mvn clean package -DskipTests

FROM eclipse-temurin:17-jre
WORKDIR /app
COPY --from=build /app/target/*.jar app.jar
EXPOSE 42000
ENTRYPOINT ["java", "-jar", "app.jar"]' > Dockerfile

# Build the Docker image
docker build -t bookmark-manager-java .

# Run the container
docker run -p 42000:42000 bookmark-manager-java
```

This will build and run the Java backend in a Docker container, accessible on port 42000.
