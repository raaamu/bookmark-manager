#!/bin/bash
# Fix the JwtAuthenticationFilter.java file

cd /Users/ramupanayappan/bookmark-manager/java-backend/src/main/java/com/example/bookmarkmanager/security

# Create a backup of the original file
cp JwtAuthenticationFilter.java JwtAuthenticationFilter.java.bak

# Modify JwtAuthenticationFilter.java to use single-parameter validateToken
sed -i '' 's/validateToken(token, userDetails)/validateToken(token)/g' JwtAuthenticationFilter.java

echo "JwtAuthenticationFilter.java has been updated."
