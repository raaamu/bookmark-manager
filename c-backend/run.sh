#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if the executable exists
if [ ! -f "./build/bookmark_manager" ]; then
    echo -e "${RED}Executable not found. Please build the project first.${NC}"
    exit 1
fi

echo -e "${BLUE}Starting C backend server...${NC}"
echo -e "${BLUE}Server will run on port 42000${NC}"
echo -e "${BLUE}Press Ctrl+C to stop the server${NC}"

# Run the server
cd build
./bookmark_manager
