#!/bin/bash

# This script cleans up Docker containers, images, and volumes related to the Java backend

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if Docker is running
echo -e "${BLUE}Checking if Docker is running...${NC}"
if ! docker info &> /dev/null; then
    echo -e "${RED}Docker is not running. Please start Docker Desktop and try again.${NC}"
    exit 1
fi

echo -e "${GREEN}Docker is running!${NC}"

# Stop and remove containers
echo -e "${BLUE}Stopping and removing containers...${NC}"
docker-compose down

# Remove Docker images
echo -e "${BLUE}Removing Docker images...${NC}"
docker rmi bookmark-manager-java 2>/dev/null || true

# Clean up build artifacts
echo -e "${BLUE}Cleaning up build artifacts...${NC}"
rm -rf target/
rm -f token.txt bookmark_id.txt

echo -e "${GREEN}Cleanup completed!${NC}"
echo -e "${YELLOW}Note: The data directory was preserved. To remove all data, delete the 'data' directory.${NC}"
