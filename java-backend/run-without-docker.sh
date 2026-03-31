#!/bin/bash

# This script runs the Java backend without Docker
# It requires Java 17+ to be installed

# Check if Java is installed
if ! command -v java &> /dev/null; then
    echo "Java is not installed. Please install Java 17 or later."
    exit 1
fi

# Check Java version
JAVA_VERSION=$(java -version 2>&1 | head -1 | cut -d'"' -f2 | sed '/^1\./s///' | cut -d'.' -f1)
if [ -z "$JAVA_VERSION" ] || [ "$JAVA_VERSION" -lt 17 ]; then
    echo "Java 17 or later is required. Current version: $JAVA_VERSION"
    exit 1
fi

# Set the active profile to test
export SPRING_PROFILES_ACTIVE=test

# Run the application using the Maven wrapper
if [ -f "./mvnw" ]; then
    ./mvnw spring-boot:run
else
    echo "Maven wrapper not found. Please run from the java-backend directory."
    exit 1
fi
