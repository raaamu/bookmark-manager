#!/bin/bash

# This script fixes permissions for the Maven wrapper script

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Fixing permissions for Maven wrapper...${NC}"

# Make Maven wrapper executable
chmod +x mvnw

echo -e "${GREEN}Permissions fixed!${NC}"
