// Step 16: Orchestrator process - AST ownership.
//
// Separate C++ process that owns the AST in memory.
// Exposes nothing yet — just loads, holds, saves on exit.
// Test: start orchestrator with Calculator.json, kill it, verify JSON saved back.

#include "Orchestrator.h"
#include <iostream>
#include <fstream>
#include <signal.h>
#include <cstdlib>
#include <thread>
#include <chrono>

// Global orchestrator instance
std::unique_ptr<Orchestrator> g_orchestrator;

// Signal handler to save on termination
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", saving and exiting..." << std::endl;
    if (g_orchestrator) {
        std::string filePath = g_orchestrator->getLoadedFilePath();
        if (!filePath.empty()) {
            g_orchestrator->saveAST(filePath);
            std::cout << "Saved AST back to: " << filePath << std::endl;
        }
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_json_file>" << std::endl;
        return 1;
    }
    
    std::string filePath = argv[1];
    
    // Set up signal handlers to save on termination
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Create orchestrator
    g_orchestrator = std::make_unique<Orchestrator>();
    
    // Load the AST from the specified file
    if (!g_orchestrator->loadAST(filePath)) {
        std::cerr << "Failed to load AST from: " << filePath << std::endl;
        return 1;
    }
    
    std::cout << "Loaded AST from: " << filePath << std::endl;
    std::cout << "Orchestrator running, holding AST in memory..." << std::endl;
    std::cout << "Press Ctrl+C to terminate and save" << std::endl;
    
    // Simple loop to keep the process alive
    // In a real implementation, this would be replaced with a proper RPC server
    while (true) {
        // Sleep briefly to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // In a real implementation, this would handle RPC requests
        // For now, we just keep the process alive to hold the AST
    }
    
    return 0;
}