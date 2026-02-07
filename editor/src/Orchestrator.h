#pragma once
#include "ast/Module.h"
#include "ast/Serialization.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>

class Orchestrator {
private:
    std::unique_ptr<Module> currentAST;
    std::string loadedFilePath;

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
};