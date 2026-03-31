#!/bin/bash

# This script installs Docker Desktop on macOS

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Installing Docker Desktop for Mac...${NC}"

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo -e "${BLUE}Installing Homebrew...${NC}"
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    
    # Add Homebrew to PATH
    if [[ $(uname -m) == "arm64" ]]; then
        echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
        eval "$(/opt/homebrew/bin/brew shellenv)"
    else
        echo 'eval "$(/usr/local/bin/brew shellenv)"' >> ~/.zprofile
        eval "$(/usr/local/bin/brew shellenv)"
    fi
fi

# Install Docker Desktop using Homebrew
echo -e "${BLUE}Installing Docker Desktop using Homebrew...${NC}"
brew install --cask docker

echo -e "${GREEN}Docker Desktop has been installed!${NC}"
echo -e "${BLUE}Please start Docker Desktop from your Applications folder.${NC}"
echo -e "${BLUE}After Docker is running, come back and run the build-and-run.sh script.${NC}"
