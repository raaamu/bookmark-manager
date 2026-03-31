cmake_minimum_required(VERSION 3.10)
project(bookmark_manager_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

# Add include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Crow/include
    ${CMAKE_CURRENT_SOURCE_DIR}/jwt-cpp/include
    /opt/homebrew/include
    ${Boost_INCLUDE_DIRS}
)

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(bookmark_manager ${SOURCES})

# Link libraries
target_link_libraries(bookmark_manager 
    PRIVATE 
    SQLite::SQLite3 
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
) 