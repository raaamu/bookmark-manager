cmake_minimum_required(VERSION 3.10)
project(bookmark_manager C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find required libraries
pkg_check_modules(MHD REQUIRED libmicrohttpd)
pkg_check_modules(JANSSON REQUIRED jansson)
pkg_check_modules(JWT REQUIRED libjwt)

# Print library paths for debugging
message(STATUS "MHD_LIBRARIES: ${MHD_LIBRARIES}")
message(STATUS "MHD_LIBRARY_DIRS: ${MHD_LIBRARY_DIRS}")
message(STATUS "JANSSON_LIBRARIES: ${JANSSON_LIBRARIES}")
message(STATUS "JWT_LIBRARIES: ${JWT_LIBRARIES}")

# Add include directories
include_directories(
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${MHD_INCLUDE_DIRS}
    ${JANSSON_INCLUDE_DIRS}
    ${JWT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add link directories explicitly before adding executable
link_directories(
    ${MHD_LIBRARY_DIRS}
    ${JANSSON_LIBRARY_DIRS}
    ${JWT_LIBRARY_DIRS}
)

# Add source files
file(GLOB SOURCES "src/*.c")

# Create executable
add_executable(bookmark_manager ${SOURCES})

# Link libraries (deduplicate to avoid duplicate symbol warnings from transitive deps)
set(ALL_LINK_LIBS
    ${SQLite3_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${MHD_LIBRARIES}
    ${JANSSON_LIBRARIES}
    ${JWT_LIBRARIES}
)
list(REMOVE_DUPLICATES ALL_LINK_LIBS)
target_link_libraries(bookmark_manager ${ALL_LINK_LIBS})

# Add compile definitions for JWT
target_compile_definitions(bookmark_manager PRIVATE
    JWT_PRIVATE_KEY="${CMAKE_SOURCE_DIR}/keys/private.pem"
    JWT_PUBLIC_KEY="${CMAKE_SOURCE_DIR}/keys/public.pem"
)
