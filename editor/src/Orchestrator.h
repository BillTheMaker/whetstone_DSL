#pragma once
#include "ast/Module.h"
#include "ast/Serialization.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include <deque>
#include <cstdio>  // For system() and popen() functions
#include "ast/Generator.h"  // For the code generators
#include "ast/Parser.h"  // For tree-sitter parser integration
#include "ast/Annotation.h"  // For annotation classes like OptimizationLock

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
    
    // Check if a node or any of its ancestors has an optimization lock
    bool isNodeLocked(const ASTNode* node) const {
        if (!node) return false;
        
        // Check if this node has an optimization lock annotation
        // In a real implementation, we'd check the annotations role for OptimizationLock nodes
        // For now, we'll implement a basic check by looking for annotations
        
        // Check this node's annotations
        auto annotations = node->getChildren("annotations");
        for (const auto* annotation : annotations) {
            if (annotation->conceptType == "OptimizationLock") {
                return true;  // Found a lock annotation on this node
            }
        }
        
        // Recursively check parent nodes
        return isNodeLocked(node->parent);
    }
    
    // Get the lock annotation for a node (if any exists in the ancestry)
    const OptimizationLock* getNodeLock(const ASTNode* node) const {
        if (!node) return nullptr;
        
        // Check if this node has an optimization lock annotation
        auto annotations = node->getChildren("annotations");
        for (const auto* annotation : annotations) {
            if (annotation->conceptType == "OptimizationLock") {
                return static_cast<const OptimizationLock*>(annotation);
            }
        }
        
        // Recursively check parent nodes
        return getNodeLock(node->parent);
    }
    
    // Get all locks on a node and its ancestors
    std::vector<const OptimizationLock*> getLocks(const std::string& nodeId) const {
        std::vector<const OptimizationLock*> locks;
        
        // Find the node by ID
        ASTNode* node = findNodeById(currentAST.get(), nodeId);
        if (!node) return locks;
        
        // Collect all locks from the node and up the ancestry
        const ASTNode* current = node;
        while (current) {
            // Check if this node has any optimization lock annotations
            auto annotations = current->getChildren("annotations");
            for (const auto* annotation : annotations) {
                if (annotation->conceptType == "OptimizationLock") {
                    locks.push_back(static_cast<const OptimizationLock*>(annotation));
                }
            }
            current = current->parent;
        }
        
        return locks;
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
                    // STUB: insertNode undo not implemented — requires role tracking per mutation
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

    // Record a snapshot (text + AST) for unified undo/redo
    void recordSnapshot(const std::string& text, const ASTNode* ast) {
        json op;
        op["type"] = "snapshot";
        op["text"] = text;
        if (ast) op["ast"] = toJson(ast);
        recordOperation(op);
    }

    // Undo snapshot
    bool undoSnapshot(std::string& outText, std::unique_ptr<Module>& outAst) {
        if (undoPosition == 0) return false;
        undoPosition--;
        json operation = operationJournal[undoPosition];
        if (operation.value("type", "") != "snapshot") return false;
        outText = operation.value("text", "");
        outAst.reset();
        if (operation.contains("ast")) {
            ASTNode* node = fromJson(operation["ast"]);
            if (node && node->conceptType == "Module") {
                outAst.reset(static_cast<Module*>(node));
            } else {
                deleteTree(node);
            }
        }
        return true;
    }

    // Redo snapshot
    bool redoSnapshot(std::string& outText, std::unique_ptr<Module>& outAst) {
        if (undoPosition >= operationJournal.size()) return false;
        json operation = operationJournal[undoPosition];
        if (operation.value("type", "") != "snapshot") return false;
        undoPosition++;
        outText = operation.value("text", "");
        outAst.reset();
        if (operation.contains("ast")) {
            ASTNode* node = fromJson(operation["ast"]);
            if (node && node->conceptType == "Module") {
                outAst.reset(static_cast<Module*>(node));
            } else {
                deleteTree(node);
            }
        }
        return true;
    }

    int getUndoDepth() const { return (int)undoPosition; }
    int getRedoDepth() const { return (int)(operationJournal.size() - undoPosition); }
    
    // Helper function to find a node by ID in the AST
    ASTNode* findNodeById(ASTNode* root, const std::string& id) const {
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
    
    // Method to start Emacs daemon
    bool startEmacsDaemon() {
        // In a real implementation, this would spawn: emacs --daemon=whetstone --load whetstone-bridge.el
        // For this implementation, we'll simulate the functionality
        // This would typically use system() or a process spawning library
        std::cout << "Starting Emacs daemon with: emacs --daemon=whetstone" << std::endl;
        // In a real implementation: system("emacs --daemon=whetstone --load whetstone-bridge.el");
        return true;
    }
    
    // Method to send a command to Emacs
    std::string sendToEmacs(const std::string& command) {
        // In a real implementation, this would send the command to Emacs via emacsclient
        // This executes: emacsclient -s whetstone -e "command" and returns the result
        std::cout << "Sending to Emacs: " << command << std::endl;
        
        // In a real implementation:
        // Escape command to prevent shell injection
        std::string escaped;
        for (char c : command) {
            if (c == '"' || c == '\\') escaped += '\\';
            escaped += c;
        }
        std::string cmd = "emacsclient -s whetstone -e \"" + escaped + "\"";
#ifdef _WIN32
        FILE* pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (!pipe) return "nil";

        char buffer[4096];
        std::string result = "";
        while (!feof(pipe)) {
            if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                result += buffer;
            }
        }
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        
        // Trim whitespace from the result
        result.erase(result.find_last_not_of(" \t\n\r\f\v") + 1);
        return result.empty() ? "nil" : result;
    }
    
    // Escape a string for embedding in Elisp double-quoted strings
    static std::string escapeElispString(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (c == '"' || c == '\\') out += '\\';
            out += c;
        }
        return out;
    }

    // Method to load a file via Emacs and parse to AST using tree-sitter
    std::unique_ptr<Module> loadFile(const std::string& path) {
        // First, get the file content from Emacs
        std::string command = "(with-current-buffer (find-file-noselect \"" + escapeElispString(path) + "\") (buffer-string))";
        std::string content = sendToEmacs(command);
        
        // Determine the language based on file extension
        std::string targetLanguage = "python";  // Default
        if (path.substr(path.find_last_of(".") + 1) == "cpp" || path.substr(path.find_last_of(".") + 1) == "hpp") {
            targetLanguage = "cpp";
        } else if (path.substr(path.find_last_of(".") + 1) == "py") {
            targetLanguage = "python";
        } else if (path.substr(path.find_last_of(".") + 1) == "el" || path.substr(path.find_last_of(".") + 1) == "elisp") {
            targetLanguage = "elisp";
        } else if (path.substr(path.find_last_of(".") + 1) == "js") {
            targetLanguage = "javascript";
        } else if (path.substr(path.find_last_of(".") + 1) == "ts") {
            targetLanguage = "typescript";
        } else if (path.substr(path.find_last_of(".") + 1) == "java") {
            targetLanguage = "java";
        }
        
        // Parse the content using the appropriate tree-sitter parser
        std::unique_ptr<Module> module;
        if (targetLanguage == "python") {
            module = TreeSitterParser::parsePython(content);
        } else if (targetLanguage == "cpp") {
            module = TreeSitterParser::parseCpp(content);
        } else if (targetLanguage == "elisp") {
            module = TreeSitterParser::parseElisp(content);
        } else if (targetLanguage == "javascript") {
            module = TreeSitterParser::parseJavaScript(content);
        } else if (targetLanguage == "typescript") {
            module = TreeSitterParser::parseTypeScript(content);
        } else if (targetLanguage == "java") {
            module = TreeSitterParser::parseJava(content);
        } else {
            // For unknown languages, create a basic module with the content
            module = std::make_unique<Module>();
            module->id = "LoadedModule_" + std::to_string(reinterpret_cast<uintptr_t>(module.get()));
            module->name = path;
            module->targetLanguage = targetLanguage;
        }
        
        // Set the module name to match the file path
        module->name = path;
        
        return module;
    }
    
    // Method to save AST to a file via Emacs using appropriate generator
    bool saveFile(const std::string& path, const ASTNode* ast) {
        if (!ast) return false;
        
        // Determine target language from the AST or file extension
        std::string targetLanguage = "python";  // Default
        if (ast->conceptType == "Module") {
            const Module* module = static_cast<const Module*>(ast);
            targetLanguage = module->targetLanguage;
        } else {
            // If not a module, try to determine from file extension
            if (path.substr(path.find_last_of(".") + 1) == "cpp" || path.substr(path.find_last_of(".") + 1) == "hpp") {
                targetLanguage = "cpp";
            } else if (path.substr(path.find_last_of(".") + 1) == "py") {
                targetLanguage = "python";
            } else if (path.substr(path.find_last_of(".") + 1) == "el" || path.substr(path.find_last_of(".") + 1) == "elisp") {
                targetLanguage = "elisp";
            } else if (path.substr(path.find_last_of(".") + 1) == "js") {
                targetLanguage = "javascript";
            } else if (path.substr(path.find_last_of(".") + 1) == "ts") {
                targetLanguage = "typescript";
            } else if (path.substr(path.find_last_of(".") + 1) == "java") {
                targetLanguage = "java";
            }
        }
        
        // Generate code from the AST using the appropriate generator
        std::string content;
        if (targetLanguage == "python") {
            PythonGenerator gen;
            content = gen.generate(ast);
        } else if (targetLanguage == "elisp") {
            ElispGenerator elispGen;
            content = elispGen.generate(ast);
        } else if (targetLanguage == "cpp" || targetLanguage == "c++") {
            CppGenerator gen;
            content = gen.generate(ast);
        } else if (targetLanguage == "javascript") {
            JavaScriptGenerator gen;
            content = gen.generate(ast);
        } else if (targetLanguage == "typescript") {
            TypeScriptGenerator gen;
            content = gen.generate(ast);
        } else if (targetLanguage == "java") {
            JavaGenerator gen;
            content = gen.generate(ast);
        } else {
            // Default to Python generator for unknown languages
            PythonGenerator gen;
            content = gen.generate(ast);
        }
        
        // Send the content to Emacs to save to file
        std::string ePath = escapeElispString(path);
        std::string eContent = escapeElispString(content);
        std::string command = "(with-current-buffer (find-file-noselect \"" + ePath + "\")" +
                             "(erase-buffer)" +
                             "(insert \"" + eContent + "\")" +
                             "(write-file \"" + ePath + "\"))";
        std::string result = sendToEmacs(command);
        return result.find("Error") == std::string::npos;
    }

    // Overload: save raw content string to a file via Emacs
    bool saveContent(const std::string& path, const std::string& content) {
        std::string ePath = escapeElispString(path);
        std::string eContent = escapeElispString(content);
        std::string command = "(with-current-buffer (find-file-noselect \"" + ePath + "\")" +
                             "(erase-buffer)" +
                             "(insert \"" + eContent + "\")" +
                             "(write-file \"" + ePath + "\"))";
        std::string result = sendToEmacs(command);
        return result.find("Error") == std::string::npos;
    }
};
