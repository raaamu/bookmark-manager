#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}Starting Angular frontend...${NC}"

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo -e "${RED}Node.js is not installed. Please install Node.js and npm.${NC}"
    exit 1
fi

# Check if npm is installed
if ! command -v npm &> /dev/null; then
    echo -e "${RED}npm is not installed. Please install npm.${NC}"
    exit 1
fi

# Install dependencies if node_modules doesn't exist or package.json has changed
if [ ! -d "node_modules" ] || [ package.json -nt node_modules ]; then
    echo -e "${BLUE}Installing dependencies...${NC}"
    npm install || { echo -e "${RED}Failed to install dependencies${NC}"; exit 1; }
fi

# Start the Angular development server
echo -e "${BLUE}Starting Angular development server...${NC}"
echo -e "${BLUE}The application will be available at http://localhost:4200${NC}"
npm start
