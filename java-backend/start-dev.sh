#!/bin/bash

# This script starts the Java backend in development mode using Docker Compose

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

# Start the application in development mode
echo -e "${BLUE}Starting Java backend in development mode...${NC}"
docker-compose -f docker-compose-dev.yml up --build

# The script will stay here while the container is running
# Press Ctrl+C to stop
