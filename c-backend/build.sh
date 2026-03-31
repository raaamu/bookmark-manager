#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}Building C backend...${NC}"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run CMake
echo -e "${BLUE}Running CMake...${NC}"
cmake .. || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

# Build the project
echo -e "${BLUE}Building project...${NC}"
make || { echo -e "${RED}Build failed${NC}"; exit 1; }

echo -e "${GREEN}Build successful!${NC}"
echo -e "${BLUE}The executable is located at:${NC} $(pwd)/bookmark_manager"
