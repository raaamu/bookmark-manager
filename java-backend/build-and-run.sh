#!/bin/bash

# This script builds and runs the Java backend using Docker

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if Docker is running
echo -e "${BLUE}Checking if Docker is running...${NC}"
if ! docker info &> /dev/null; then
    echo -e "${RED}Docker is not running. Please start Docker Desktop and try again.${NC}"
    exit 1
fi

echo -e "${GREEN}Docker is running!${NC}"

# Create .mvn directory structure if it doesn't exist
if [ ! -d ".mvn/wrapper" ]; then
    echo -e "${BLUE}Creating Maven wrapper directory structure...${NC}"
    mkdir -p .mvn/wrapper
fi

# Build the Docker image
echo -e "${BLUE}Building Docker image...${NC}"
docker build -t bookmark-manager-java .

# Run the Docker container
echo -e "${BLUE}Running Docker container...${NC}"
docker run -d -p 42000:42000 --name bookmark-manager-java bookmark-manager-java

# Check if container is running
if docker ps | grep -q bookmark-manager-java; then
    echo -e "${GREEN}Java backend is running on port 42000!${NC}"
    echo -e "${BLUE}You can test the API using the test-api.sh script.${NC}"
else
    echo -e "${RED}Failed to start the container. Check Docker logs:${NC}"
    docker logs bookmark-manager-java
fi
