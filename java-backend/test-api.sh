#!/bin/bash

# This script tests the Java backend API endpoints

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# API base URL
API_URL="http://localhost:42000/api"
AUTH_URL="${API_URL}/auth"
BOOKMARKS_URL="${API_URL}/bookmarks"

# Test user credentials
USERNAME="testuser"
PASSWORD="password123"
EMAIL="test@example.com"

# Function to check if the API is available
check_api() {
    echo -e "${BLUE}Checking if API is available...${NC}"
    if curl -s --head "${API_URL}/actuator/health" | grep "200 OK" > /dev/null; then
        echo -e "${GREEN}API is available!${NC}"
        return 0
    else
        echo -e "${RED}API is not available. Make sure the backend is running.${NC}"
        return 1
    fi
}

# Function to register a new user
register_user() {
    echo -e "${BLUE}Registering new user...${NC}"
    RESPONSE=$(curl -s -X POST "${AUTH_URL}/register" \
        -H "Content-Type: application/json" \
        -d "{\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"email\":\"${EMAIL}\"}")
    
    if echo "$RESPONSE" | grep -q "User registered successfully"; then
        echo -e "${GREEN}User registered successfully!${NC}"
        return 0
    else
        echo -e "${YELLOW}User might already exist or registration failed: ${RESPONSE}${NC}"
        return 0  # Continue anyway
    fi
}

# Function to login and get JWT token
login_user() {
    echo -e "${BLUE}Logging in...${NC}"
    RESPONSE=$(curl -s -X POST "${AUTH_URL}/login" \
        -H "Content-Type: application/json" \
        -d "{\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\"}")
    
    TOKEN=$(echo "$RESPONSE" | grep -o '"token":"[^"]*' | sed 's/"token":"//')
    
    if [ -z "$TOKEN" ]; then
        echo -e "${RED}Login failed: ${RESPONSE}${NC}"
        return 1
    else
        echo -e "${GREEN}Login successful! Token received.${NC}"
        echo "$TOKEN" > token.txt
        return 0
    fi
}

# Function to get all bookmarks
get_bookmarks() {
    echo -e "${BLUE}Getting all bookmarks...${NC}"
    TOKEN=$(cat token.txt)
    RESPONSE=$(curl -s -X GET "${BOOKMARKS_URL}" \
        -H "Authorization: Bearer ${TOKEN}")
    
    echo -e "${GREEN}Bookmarks: ${RESPONSE}${NC}"
    return 0
}

# Function to create a bookmark
create_bookmark() {
    echo -e "${BLUE}Creating a bookmark...${NC}"
    TOKEN=$(cat token.txt)
    RESPONSE=$(curl -s -X POST "${BOOKMARKS_URL}" \
        -H "Content-Type: application/json" \
        -H "Authorization: Bearer ${TOKEN}" \
        -d '{"title":"Test Bookmark","url":"https://example.com","description":"This is a test bookmark"}')
    
    BOOKMARK_ID=$(echo "$RESPONSE" | grep -o '"id":[0-9]*' | sed 's/"id"://')
    
    if [ -z "$BOOKMARK_ID" ]; then
        echo -e "${RED}Failed to create bookmark: ${RESPONSE}${NC}"
        return 1
    else
        echo -e "${GREEN}Bookmark created with ID: ${BOOKMARK_ID}${NC}"
        echo "$BOOKMARK_ID" > bookmark_id.txt
        return 0
    fi
}

# Function to search bookmarks
search_bookmarks() {
    echo -e "${BLUE}Searching bookmarks with term 'test'...${NC}"
    TOKEN=$(cat token.txt)
    RESPONSE=$(curl -s -X GET "${BOOKMARKS_URL}?q=test" \
        -H "Authorization: Bearer ${TOKEN}")
    
    echo -e "${GREEN}Search results: ${RESPONSE}${NC}"
    return 0
}

# Function to delete a bookmark
delete_bookmark() {
    if [ ! -f bookmark_id.txt ]; then
        echo -e "${RED}No bookmark ID found. Create a bookmark first.${NC}"
        return 1
    fi
    
    BOOKMARK_ID=$(cat bookmark_id.txt)
    echo -e "${BLUE}Deleting bookmark with ID: ${BOOKMARK_ID}...${NC}"
    
    TOKEN=$(cat token.txt)
    RESPONSE=$(curl -s -X DELETE "${BOOKMARKS_URL}/${BOOKMARK_ID}" \
        -H "Authorization: Bearer ${TOKEN}")
    
    echo -e "${GREEN}Bookmark deleted: ${RESPONSE}${NC}"
    rm bookmark_id.txt
    return 0
}

# Main test flow
main() {
    echo -e "${BLUE}Starting API tests...${NC}"
    
    # Check if API is available
    check_api || exit 1
    
    # Register a new user
    register_user
    
    # Login and get token
    login_user || exit 1
    
    # Get initial bookmarks (should be empty)
    get_bookmarks
    
    # Create a bookmark
    create_bookmark || exit 1
    
    # Get bookmarks again (should include the new bookmark)
    get_bookmarks
    
    # Search bookmarks
    search_bookmarks
    
    # Delete the bookmark
    delete_bookmark
    
    # Get bookmarks again (should be empty)
    get_bookmarks
    
    # Clean up
    rm -f token.txt bookmark_id.txt
    
    echo -e "${GREEN}All tests completed successfully!${NC}"
}

# Run the tests
main
