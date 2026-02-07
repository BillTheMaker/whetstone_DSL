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

// Forward declarations for helper functions
ASTNode* findNodeById(ASTNode* root, const std::string& id);
bool setNodeProperty(ASTNode* node, const std::string& property, const json& value);
void setNodeProperties(ASTNode* node, const json& properties);
ASTNode* createASTNode(const std::string& conceptType);
bool isRoleValid(ASTNode* parent, const std::string& role, ASTNode* child);
std::string generateUniqueId();

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
        else if (method == "setNodeProperty") {
            try {
                std::string nodeId = request.at("params").at("nodeId");
                std::string property = request.at("params").at("property");
                json value = request.at("params").at("value");
                
                // Find the node by ID in the AST
                ASTNode* node = findNodeById(g_orchestrator->getAST(), nodeId);
                if (!node) {
                    response["error"] = json::object({
                        {"code", -32602},
                        {"message", "Node not found: " + nodeId}
                    });
                } else {
                    // Set the property on the node
                    bool success = setNodeProperty(node, property, value);
                    if (success) {
                        response["result"] = true;
                    } else {
                        response["error"] = json::object({
                            {"code", -32603},
                            {"message", "Cannot set property '" + property + "' on node type '" + node->conceptType + "'"}
                        });
                    }
                }
            } catch (...) {
                response["error"] = json::object({
                    {"code", -32600},
                    {"message", "Invalid parameters for setNodeProperty"}
                });
            }
        }
        else if (method == "insertNode") {
            try {
                std::string parentId = request.at("params").at("parentId");
                std::string role = request.at("params").at("role");
                std::string conceptType = request.at("params").at("concept");
                json properties = request.at("params").at("properties");
                
                // Find the parent node by ID in the AST
                ASTNode* parentNode = findNodeById(g_orchestrator->getAST(), parentId);
                if (!parentNode) {
                    response["error"] = json::object({
                        {"code", -32602},
                        {"message", "Parent node not found: " + parentId}
                    });
                } else {
                    // Create and insert the new node
                    ASTNode* newNode = createASTNode(conceptType);
                    if (!newNode) {
                        response["error"] = json::object({
                            {"code", -32603},
                            {"message", "Unknown concept: " + conceptType}
                        });
                    } else {
                        // Set the node ID and properties
                        newNode->id = generateUniqueId(); // Generate a unique ID
                        setNodeProperties(newNode, properties);
                        
                        // Add to parent in the specified role
                        if (isRoleValid(parentNode, role, newNode)) {
                            parentNode->addChild(role, newNode);
                            
                            // Record the operation in the journal for undo/redo
                            json operation = json::object({
                                {"type", "insertNode"},
                                {"nodeId", newNode->id},
                                {"parentId", parentId},
                                {"role", role},
                                {"concept", conceptType},
                                {"properties", properties}
                            });
                            g_orchestrator->recordOperation(operation);
                            
                            response["result"] = toJson(newNode); // Return the inserted node as JSON
                        } else {
                            delete newNode; // Clean up the node if insertion failed
                            response["error"] = json::object({
                                {"code", -32603},
                                {"message", "Invalid role '" + role + "' for concept '" + parentNode->conceptType + 
                                           "' with child concept '" + newNode->conceptType + "'"}
                            });
                        }
                    }
                }
            } catch (...) {
                response["error"] = json::object({
                    {"code", -32600},
                    {"message", "Invalid parameters for insertNode"}
                });
            }
        }
        else if (method == "setNodeProperty") {
            try {
                std::string nodeId = request.at("params").at("nodeId");
                std::string property = request.at("params").at("property");
                json value = request.at("params").at("value");
                
                // Get the old value for undo
                ASTNode* node = findNodeById(g_orchestrator->getAST(), nodeId);
                if (!node) {
                    response["error"] = json::object({
                        {"code", -32602},
                        {"message", "Node not found: " + nodeId}
                    });
                } else {
                    // Store old value for undo
                    json oldValue;
                    if (node->conceptType == "Function" && property == "name") {
                        oldValue = static_cast<Function*>(node)->name;
                    } else if (node->conceptType == "Variable" && property == "name") {
                        oldValue = static_cast<Variable*>(node)->name;
                    } else if (node->conceptType == "Parameter" && property == "name") {
                        oldValue = static_cast<Parameter*>(node)->name;
                    } else {
                        response["error"] = json::object({
                            {"code", -32603},
                            {"message", "Cannot set property '" + property + "' on node type '" + node->conceptType + "'"}
                        });
                        return response;
                    }
                    
                    // Set the property on the node
                    bool success = setNodeProperty(node, property, value);
                    if (success) {
                        // Record the operation in the journal for undo/redo
                        json operation = json::object({
                            {"type", "setNodeProperty"},
                            {"nodeId", nodeId},
                            {"property", property},
                            {"oldValue", oldValue},
                            {"newValue", value}
                        });
                        g_orchestrator->recordOperation(operation);
                        
                        response["result"] = true;
                    } else {
                        response["error"] = json::object({
                            {"code", -32603},
                            {"message", "Cannot set property '" + property + "' on node type '" + node->conceptType + "'"}
                        });
                    }
                }
            } catch (...) {
                response["error"] = json::object({
                    {"code", -32600},
                    {"message", "Invalid parameters for setNodeProperty"}
                });
            }
        }
        else if (method == "undo") {
            try {
                bool success = g_orchestrator->undoLastOperation();
                if (success) {
                    response["result"] = true;
                } else {
                    response["error"] = json::object({
                        {"code", -32603},
                        {"message", "No operations to undo"}
                    });
                }
            } catch (...) {
                response["error"] = json::object({
                    {"code", -32600},
                    {"message", "Error during undo operation"}
                });
            }
        }
        else if (method == "redo") {
            try {
                bool success = g_orchestrator->redoLastOperation();
                if (success) {
                    response["result"] = true;
                } else {
                    response["error"] = json::object({
                        {"code", -32603},
                        {"message", "No operations to redo"}
                    });
                }
            } catch (...) {
                response["error"] = json::object({
                    {"code", -32600},
                    {"message", "Error during redo operation"}
                });
            }
        }
        else if (method == "sendToEmacs") {
            try {
                std::string command = request.at("params").at("command");
                std::string result = g_orchestrator->sendToEmacs(command);
                response["result"] = result;
            } catch (...) {
                response["error"] = json::object({
                    {"code", -32600},
                    {"message", "Invalid parameters for sendToEmacs"}
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

// Helper function to find a node by ID in the AST
ASTNode* findNodeById(ASTNode* root, const std::string& id) {
    if (!root) return nullptr;
    if (root->id == id) return root;
    
    // Recursively search in children
    for (auto* child : root->allChildren()) {
        ASTNode* found = findNodeById(child, id);
        if (found) return found;
    }
    
    return nullptr;
}

// Helper function to set a property on a node
bool setNodeProperty(ASTNode* node, const std::string& property, const json& value) {
    // This is a simplified implementation - in a real system, we'd have specific setters for each concept
    // For now, we'll handle a few common properties
    if (node->conceptType == "Function" && property == "name") {
        if (value.is_string()) {
            static_cast<Function*>(node)->name = value.get<std::string>();
            return true;
        }
    } else if (node->conceptType == "Variable" && property == "name") {
        if (value.is_string()) {
            static_cast<Variable*>(node)->name = value.get<std::string>();
            return true;
        }
    } else if (node->conceptType == "Parameter" && property == "name") {
        if (value.is_string()) {
            static_cast<Parameter*>(node)->name = value.get<std::string>();
            return true;
        }
    }
    // Add more property setters as needed
    
    return false; // Property not supported
}

// Helper function to set multiple properties on a node
void setNodeProperties(ASTNode* node, const json& properties) {
    for (auto& [propName, propValue] : properties.items()) {
        setNodeProperty(node, propName, propValue);
    }
}

// Helper function to set a single property on a node (separate implementation to avoid recursion)
bool setNodePropertyDirect(ASTNode* node, const std::string& property, const json& value) {
    // This is a simplified implementation - in a real system, we'd have specific setters for each concept
    // For now, we'll handle a few common properties
    if (node->conceptType == "Function" && property == "name") {
        if (value.is_string()) {
            static_cast<Function*>(node)->name = value.get<std::string>();
            return true;
        }
    } else if (node->conceptType == "Variable" && property == "name") {
        if (value.is_string()) {
            static_cast<Variable*>(node)->name = value.get<std::string>();
            return true;
        }
    } else if (node->conceptType == "Parameter" && property == "name") {
        if (value.is_string()) {
            static_cast<Parameter*>(node)->name = value.get<std::string>();
            return true;
        }
    }
    // Add more property setters as needed
    
    return false; // Property not supported
}

// Helper function to create a new node of the specified concept
ASTNode* createASTNode(const std::string& conceptType) {
    if (conceptType == "Function") return new Function();
    if (conceptType == "Variable") return new Variable();
    if (conceptType == "Parameter") return new Parameter();
    if (conceptType == "Assignment") return new Assignment();
    if (conceptType == "Return") return new Return();
    if (conceptType == "BinaryOperation") return new BinaryOperation();
    if (conceptType == "VariableReference") return new VariableReference();
    if (conceptType == "IntegerLiteral") return new IntegerLiteral();
    if (conceptType == "StringLiteral") return new StringLiteral();
    if (conceptType == "BooleanLiteral") return new BooleanLiteral();
    if (conceptType == "PrimitiveType") return new PrimitiveType();
    // Add more concepts as needed
    
    return nullptr;
}

// Helper function to check if a role is valid for a parent-child relationship
bool isRoleValid(ASTNode* parent, const std::string& role, ASTNode* child) {
    // This is a simplified implementation - in a real system, we'd have a schema to check validity
    // For now, we'll allow common roles based on parent concept
    if (parent->conceptType == "Module") {
        return (role == "functions" || role == "variables" || role == "annotations");
    } else if (parent->conceptType == "Function") {
        return (role == "parameters" || role == "body" || role == "annotations" || role == "returnType");
    } else if (parent->conceptType == "Assignment") {
        return (role == "target" || role == "value");
    } else if (parent->conceptType == "BinaryOperation") {
        return (role == "left" || role == "right");
    }
    // Add more role validations as needed
    
    return false; // Role not supported
}

// Helper function to generate a unique ID
std::string generateUniqueId() {
    static int counter = 0;
    return "RPC_" + std::to_string(++counter);
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
    
    // Start the Emacs daemon
    g_orchestrator->startEmacsDaemon();
    
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