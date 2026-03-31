#!/bin/bash

# This script runs the Java backend using Docker
# It doesn't require Java to be installed locally

echo "Starting Java backend using Docker..."
echo "The application will be available at http://localhost:42000"

# Create data directory if it doesn't exist
mkdir -p data

# Run the application using Docker
docker run --rm -p 42000:42000 \
  -v $(pwd):/app \
  -w /app \
  openjdk:17-slim \
  ./mvnw spring-boot:run
