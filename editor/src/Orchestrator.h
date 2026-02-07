#pragma once
#include "ast/Module.h"
#include "ast/Serialization.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include <deque>

class Orchestrator {
private:
    std::unique_ptr<Module> currentAST;
    std::string loadedFilePath;
    
    // Operation journal for undo/redo
    std::deque<json> operationJournal;
    size_t undoPosition = 0;  // Points to the next operation to undo (0 = no operations to undo)

public:
    Orchestrator() = default;
    
    ~Orchestrator() {
        // On destruction, save the AST back if there's a loaded file
        if (!loadedFilePath.empty() && currentAST) {
            saveAST(loadedFilePath);
        }
    }
    
    // Load AST from JSON file
    bool loadAST(const std::string& filePath) {
        try {
            // Read the file
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return false;
            }
            
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();
            
            // Parse JSON
            auto j = json::parse(content);
            
            // Deserialize to AST
            ASTNode* node = fromJson(j);
            if (!node || node->conceptType != "Module") {
                deleteTree(node);
                return false;
            }
            
            currentAST.reset(static_cast<Module*>(node));
            loadedFilePath = filePath;
            return true;
        } catch (...) {
            return false;
        }
    }
    
    // Save AST to JSON file
    bool saveAST(const std::string& filePath) {
        if (!currentAST) {
            return false;
        }
        
        try {
            json j = toJson(currentAST.get());
            std::ofstream file(filePath);
            if (!file.is_open()) {
                return false;
            }
            
            file << j.dump(2);
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    // Get current AST
    Module* getAST() {
        return currentAST.get();
    }
    
    // Set current AST
    void setAST(std::unique_ptr<Module> ast) {
        currentAST = std::move(ast);
    }
    
    // Get loaded file path
    std::string getLoadedFilePath() const {
        return loadedFilePath;
    }
    
    // Record an operation for undo/redo
    void recordOperation(const json& operation) {
        // Clear any operations after the current position (if we're in the middle of the undo stack)
        while (operationJournal.size() > undoPosition) {
            operationJournal.pop_back();
        }
        
        // Add the new operation
        operationJournal.push_back(operation);
        undoPosition = operationJournal.size();  // Point to after the last operation
    }
    
    // Undo the last operation
    bool undoLastOperation() {
        if (undoPosition == 0) {
            return false;  // Nothing to undo
        }
        
        // Move back one operation
        undoPosition--;
        json operation = operationJournal[undoPosition];
        
        // Perform the inverse of the operation
        if (operation["type"] == "insertNode") {
            // For insertNode, the inverse is to delete the node
            std::string nodeId = operation["nodeId"];
            ASTNode* node = findNodeById(currentAST.get(), nodeId);
            if (node) {
                // Find the parent and remove the child
                ASTNode* parent = node->parent;
                if (parent) {
                    // This is a simplified implementation - in reality, we'd need to track
                    // which role the node was added to and remove it from that role
                    // For now, we'll just mark this as a TODO in a real implementation
                }
            }
        } else if (operation["type"] == "setNodeProperty") {
            // For setNodeProperty, the inverse is to set the property back to its old value
            std::string nodeId = operation["nodeId"];
            std::string property = operation["property"];
            json oldValue = operation["oldValue"];
            
            ASTNode* node = findNodeById(currentAST.get(), nodeId);
            if (node) {
                setNodeProperty(node, property, oldValue);
            }
        }
        
        return true;
    }
    
    // Redo the last undone operation
    bool redoLastOperation() {
        if (undoPosition >= operationJournal.size()) {
            return false;  // Nothing to redo
        }
        
        // Perform the operation
        json operation = operationJournal[undoPosition];
        if (operation["type"] == "insertNode") {
            // Re-insert the node
            // This would require recreating the node and adding it back - simplified for this example
        } else if (operation["type"] == "setNodeProperty") {
            // Re-set the property to its new value
            std::string nodeId = operation["nodeId"];
            std::string property = operation["property"];
            json newValue = operation["newValue"];
            
            ASTNode* node = findNodeById(currentAST.get(), nodeId);
            if (node) {
                setNodeProperty(node, property, newValue);
            }
        }
        
        // Move forward one operation
        undoPosition++;
        return true;
    }
    
    // Helper function to find a node by ID in the AST (needs to be implemented)
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
    
    // Helper function to set a property on a node (needs to be implemented)
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
};