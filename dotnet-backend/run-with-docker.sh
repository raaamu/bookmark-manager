#!/bin/bash

# Build the Docker image
docker build -t bookmark-manager-dotnet .

# Run the container
docker run -d -p 42000:42000 --name bookmark-manager-dotnet-container bookmark-manager-dotnet

echo "Bookmark Manager .NET backend is running at http://localhost:42000"
