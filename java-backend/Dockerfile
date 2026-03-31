FROM openjdk:17-slim AS build

WORKDIR /app

# Copy Maven wrapper and pom.xml
COPY .mvn/ .mvn/
COPY mvnw pom.xml ./

# Make the Maven wrapper executable
RUN chmod +x ./mvnw

# Download dependencies
RUN ./mvnw dependency:go-offline -B

# Copy source code
COPY src ./src/

# Build the application
RUN ./mvnw package -DskipTests

# Runtime stage
FROM openjdk:17-slim

WORKDIR /app

# Copy the built JAR file from the build stage
COPY --from=build /app/target/bookmark-manager-0.0.1-SNAPSHOT.jar ./app.jar

# Create a directory for data
RUN mkdir -p /app/data

# Run the application
EXPOSE 42000
ENTRYPOINT ["java", "-jar", "app.jar"]
