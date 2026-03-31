#include "../include/app.h"
#include <signal.h>

static volatile int keep_running = 1;

static void handle_signal(int signal) {
    keep_running = 0;
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default port 8080.\n");
            port = 8080;
        }
    }
    
    ErrorCode result = start_server(port);
    if (result != SUCCESS) {
        fprintf(stderr, "Failed to start server: %s\n", get_error_message(result));
        return 1;
    }
    
    printf("Bookmark Manager API server running on port %d\n", port);
    printf("Press Ctrl+C to stop the server\n");
    
    // Keep the main thread alive
    while (keep_running) {
        sleep(1);
    }
    
    printf("\nShutting down server...\n");
    
    return 0;
}
