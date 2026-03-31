#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

API_URL="http://localhost:42000"
TOKEN=""
USER_EMAIL="test@example.com"
USER_PASSWORD="password123"
USER_USERNAME="testuser"

# Function to print section header
print_header() {
    echo -e "\n${BLUE}==== $1 ====${NC}"
}

# Function to check if the API server is running
check_server() {
    print_header "Checking if API server is running"
    if curl -s --head --request GET "$API_URL" | grep "200 OK" > /dev/null; then
        echo -e "${GREEN}API server is running at $API_URL${NC}"
    else
        echo -e "${RED}API server is not running at $API_URL${NC}"
        echo -e "${YELLOW}Please start the server before running this script${NC}"
        exit 1
    fi
}

# Function to register a new user
register_user() {
    print_header "Registering a new user"
    
    RESPONSE=$(curl -s -X POST "$API_URL/register" \
        -H "Content-Type: application/json" \
        -d "{\"username\":\"$USER_USERNAME\",\"email\":\"$USER_EMAIL\",\"password\":\"$USER_PASSWORD\"}")
    
    if [[ $RESPONSE == *"success"* ]]; then
        echo -e "${GREEN}User registered successfully${NC}"
    else
        echo -e "${YELLOW}User registration response: $RESPONSE${NC}"
        echo -e "${YELLOW}User might already exist, proceeding with login${NC}"
    fi
}

# Function to login and get JWT token
login_user() {
    print_header "Logging in to get JWT token"
    
    RESPONSE=$(curl -s -X POST "$API_URL/login" \
        -H "Content-Type: application/json" \
        -d "{\"username\":\"$USER_USERNAME\",\"password\":\"$USER_PASSWORD\"}")
    
    if [[ $RESPONSE == *"token"* ]]; then
        TOKEN=$(echo $RESPONSE | sed -n 's/.*"token":"\([^"]*\)".*/\1/p')
        echo -e "${GREEN}Login successful${NC}"
        echo -e "${GREEN}Token: ${TOKEN:0:20}...${NC}"
    else
        echo -e "${RED}Login failed: $RESPONSE${NC}"
        exit 1
    fi
}

# Function to get all bookmarks
get_bookmarks() {
    print_header "Getting all bookmarks"
    
    RESPONSE=$(curl -s -X GET "$API_URL/bookmarks" \
        -H "Authorization: Bearer $TOKEN")
    
    echo -e "${GREEN}Bookmarks:${NC}"
    echo $RESPONSE | python -m json.tool
}

# Function to create a new bookmark
create_bookmark() {
    print_header "Creating a new bookmark"
    
    RESPONSE=$(curl -s -X POST "$API_URL/bookmarks" \
        -H "Content-Type: application/json" \
        -H "Authorization: Bearer $TOKEN" \
        -d "{\"title\":\"Test Bookmark\",\"url\":\"https://example.com\",\"description\":\"This is a test bookmark\"}")
    
    if [[ $RESPONSE == *"id"* ]]; then
        BOOKMARK_ID=$(echo $RESPONSE | sed -n 's/.*"id":\([0-9]*\).*/\1/p')
        echo -e "${GREEN}Bookmark created successfully with ID: $BOOKMARK_ID${NC}"
    else
        echo -e "${RED}Failed to create bookmark: $RESPONSE${NC}"
    fi
}

# Function to search bookmarks
search_bookmarks() {
    print_header "Searching bookmarks"
    
    RESPONSE=$(curl -s -X GET "$API_URL/bookmarks?q=test" \
        -H "Authorization: Bearer $TOKEN")
    
    echo -e "${GREEN}Search results:${NC}"
    echo $RESPONSE | python -m json.tool
}

# Function to delete a bookmark
delete_bookmark() {
    print_header "Deleting a bookmark"
    
    # First get all bookmarks to find one to delete
    BOOKMARKS=$(curl -s -X GET "$API_URL/bookmarks" \
        -H "Authorization: Bearer $TOKEN")
    
    # Extract the first bookmark ID
    BOOKMARK_ID=$(echo $BOOKMARKS | sed -n 's/.*"id":\([0-9]*\).*/\1/p' | head -1)
    
    if [ -z "$BOOKMARK_ID" ]; then
        echo -e "${YELLOW}No bookmarks found to delete${NC}"
        return
    fi
    
    RESPONSE=$(curl -s -X DELETE "$API_URL/bookmarks/$BOOKMARK_ID" \
        -H "Authorization: Bearer $TOKEN")
    
    if [[ $RESPONSE == *"success"* ]]; then
        echo -e "${GREEN}Bookmark with ID $BOOKMARK_ID deleted successfully${NC}"
    else
        echo -e "${RED}Failed to delete bookmark: $RESPONSE${NC}"
    fi
}

# Main execution
check_server
register_user
login_user
get_bookmarks
create_bookmark
search_bookmarks
get_bookmarks
delete_bookmark
get_bookmarks

echo -e "\n${GREEN}API testing completed successfully!${NC}"
