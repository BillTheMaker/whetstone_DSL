// Step 17: JSON-RPC server - Ping and getAST methods.
//
// Orchestrator listens on stdin/stdout for JSON-RPC.
// Responds to `ping` and `getAST` methods.
// Test: send JSON-RPC from Python script, get response.

#include "Orchestrator.h"
#include <iostream>
#include <fstream>
#include <signal.h>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

// Process a JSON-RPC request
json processRequest(const json& request) {
    json response = json::object();
    response["jsonrpc"] = "2.0";
    response["id"] = request.contains("id") ? request["id"] : json(nullptr);
    
    try {
        std::string method = request.at("method");
        
        if (method == "ping") {
            response["result"] = "pong";
        }
        else if (method == "getAST") {
            if (g_orchestrator && g_orchestrator->getAST()) {
                // Serialize the AST to JSON
                json astJson = toJson(g_orchestrator->getAST());
                response["result"] = astJson;
            } else {
                response["error"] = json::object({
                    {"code", -32603},
                    {"message", "No AST loaded"}
                });
            }
        }
        else {
            response["error"] = json::object({
                {"code", -32601},
                {"message", "Method not found: " + method}
            });
        }
    } catch (const std::exception& e) {
        response["error"] = json::object({
            {"code", -32600},
            {"message", "Invalid Request: " + std::string(e.what())}
        });
    } catch (...) {
        response["error"] = json::object({
            {"code", -32600},
            {"message", "Invalid Request"}
        });
    }
    
    return response;
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
    std::cout << "Orchestrator running with JSON-RPC server..." << std::endl;
    std::cout << "Listening for JSON-RPC requests on stdin..." << std::endl;
    
    // Main loop to handle JSON-RPC requests from stdin
    std::string line;
    while (std::getline(std::cin, line)) {
        try {
            // Parse the incoming JSON-RPC request
            json request = json::parse(line);
            
            // Process the request
            json response = processRequest(request);
            
            // Send the response to stdout
            std::cout << response.dump() << std::endl;
            std::cout.flush();
        } catch (const std::exception& e) {
            // Send error response if request parsing fails
            json errorResponse = json::object({
                {"jsonrpc", "2.0"},
                {"id", json(nullptr)}, // Use null if we couldn't parse the original id
                {"error", json::object({
                    {"code", -32700},
                    {"message", "Parse error: " + std::string(e.what())}
                })}
            });
            std::cout << errorResponse.dump() << std::endl;
            std::cout.flush();
        }
    }
    
    return 0;
}