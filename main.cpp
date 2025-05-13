// main.cpp
// Author: JJ

#include "app.hpp"
#include <chrono>
#include <iostream>

int main() {
    auto start = std::chrono::steady_clock::now(); // Start time measurement

    App app;
    if (!app.init()) {
        return EXIT_FAILURE;
    }
    int result = app.run();  // Run the application

    auto end = std::chrono::steady_clock::now(); // End time measurement

    // Compute and print elapsed time
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << " sec" << std::endl;

    return result;
}
