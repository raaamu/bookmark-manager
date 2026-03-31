#!/bin/bash

# This script starts both the Java backend and Angular frontend

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

# Fix permissions for Maven wrapper
./fix-permissions.sh

# Create data directory if it doesn't exist
mkdir -p data

# Start the backend in production mode
echo -e "${BLUE}Starting Java backend in production mode...${NC}"
docker-compose up -d --build

# Check if backend started successfully
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to start the backend. Check Docker logs.${NC}"
    exit 1
fi

echo -e "${GREEN}Java backend started successfully on port 42000${NC}"

# Check if Angular frontend exists
if [ ! -d "../frontend/bookmark-manager-frontend" ]; then
    echo -e "${RED}Angular frontend directory not found.${NC}"
    echo -e "${YELLOW}Backend is running, but frontend could not be started.${NC}"
    exit 1
fi

# Start the Angular frontend
echo -e "${BLUE}Starting Angular frontend...${NC}"
cd ../frontend/bookmark-manager-frontend

# Check if npm is installed
if ! command -v npm &> /dev/null; then
    echo -e "${RED}npm is not installed. Cannot start frontend.${NC}"
    echo -e "${YELLOW}Backend is running, but frontend could not be started.${NC}"
    exit 1
fi

# Install dependencies if node_modules doesn't exist
if [ ! -d "node_modules" ]; then
    echo -e "${BLUE}Installing frontend dependencies...${NC}"
    npm install
fi

# Start the Angular application
echo -e "${BLUE}Starting Angular development server...${NC}"
npm start

# The script will stay here while the Angular server is running
# Press Ctrl+C to stop
