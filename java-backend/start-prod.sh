#!/bin/bash

# This script starts the Java backend in production mode using Docker Compose

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

# Fix permissions for Maven wrapper
./fix-permissions.sh

# Create data directory if it doesn't exist
mkdir -p data

# Start the application in production mode
echo -e "${BLUE}Starting Java backend in production mode...${NC}"
docker-compose up -d --build

echo -e "${GREEN}Java backend started in production mode on port 42000${NC}"
echo -e "${BLUE}To view logs: docker-compose logs -f${NC}"
echo -e "${BLUE}To stop: docker-compose down${NC}"
