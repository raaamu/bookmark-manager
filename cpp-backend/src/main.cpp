#include "app.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        BookmarkManager app;
        app.run(5001);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 