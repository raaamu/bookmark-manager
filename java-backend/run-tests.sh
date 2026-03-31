#!/bin/bash

# This script runs manual tests against the Java backend API

# Base URL
BASE_URL="http://localhost:42000"

# Test user credentials
USERNAME="testuser"
PASSWORD="password123"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Starting manual API tests for Java Bookmark Manager Backend${NC}"
echo "---------------------------------------------------"

# 1. Register a new user
echo -e "\n${BLUE}1. Registering a new user...${NC}"
REGISTER_RESPONSE=$(curl -s -X POST "$BASE_URL/register" \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD\"}")

echo "Response: $REGISTER_RESPONSE"

# 2. Login with the user
echo -e "\n${BLUE}2. Logging in with the user...${NC}"
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/login" \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD\"}")

echo "Response: $LOGIN_RESPONSE"

# Extract token from login response
TOKEN=$(echo $LOGIN_RESPONSE | sed -n 's/.*"token":"\([^"]*\)".*/\1/p')

if [ -z "$TOKEN" ]; then
  echo -e "${RED}Failed to extract token from login response${NC}"
  exit 1
fi

echo -e "${GREEN}Successfully extracted token${NC}"

# 3. Get bookmarks (initially empty)
echo -e "\n${BLUE}3. Getting bookmarks (should be empty)...${NC}"
BOOKMARKS_RESPONSE=$(curl -s -X GET "$BASE_URL/bookmarks" \
  -H "Authorization: Bearer $TOKEN")

echo "Response: $BOOKMARKS_RESPONSE"

# 4. Create a bookmark
echo -e "\n${BLUE}4. Creating a bookmark...${NC}"
CREATE_RESPONSE=$(curl -s -X POST "$BASE_URL/bookmarks" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"Test Bookmark","url":"https://example.com","description":"This is a test bookmark"}')

echo "Response: $CREATE_RESPONSE"

# Extract bookmark ID
BOOKMARK_ID=$(echo $CREATE_RESPONSE | sed -n 's/.*"id":\([0-9]*\).*/\1/p')

if [ -z "$BOOKMARK_ID" ]; then
  echo -e "${RED}Failed to extract bookmark ID from create response${NC}"
  exit 1
fi

echo -e "${GREEN}Successfully created bookmark with ID: $BOOKMARK_ID${NC}"

# 5. Get bookmarks (should contain the created bookmark)
echo -e "\n${BLUE}5. Getting bookmarks (should contain the created bookmark)...${NC}"
BOOKMARKS_RESPONSE=$(curl -s -X GET "$BASE_URL/bookmarks" \
  -H "Authorization: Bearer $TOKEN")

echo "Response: $BOOKMARKS_RESPONSE"

# 6. Search bookmarks
echo -e "\n${BLUE}6. Searching bookmarks with query 'test'...${NC}"
SEARCH_RESPONSE=$(curl -s -X GET "$BASE_URL/bookmarks?q=test" \
  -H "Authorization: Bearer $TOKEN")

echo "Response: $SEARCH_RESPONSE"

# 7. Delete the bookmark
echo -e "\n${BLUE}7. Deleting the bookmark...${NC}"
DELETE_RESPONSE=$(curl -s -X DELETE "$BASE_URL/bookmarks/$BOOKMARK_ID" \
  -H "Authorization: Bearer $TOKEN")

echo "Response: $DELETE_RESPONSE"

# 8. Get bookmarks after deletion (should be empty)
echo -e "\n${BLUE}8. Getting bookmarks after deletion (should be empty)...${NC}"
BOOKMARKS_RESPONSE=$(curl -s -X GET "$BASE_URL/bookmarks" \
  -H "Authorization: Bearer $TOKEN")

echo "Response: $BOOKMARKS_RESPONSE"

echo -e "\n${GREEN}Manual API tests completed!${NC}"
