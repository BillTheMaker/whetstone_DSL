#pragma once
// Step 62: Context API
//
// ContextAPI: provides scope, call-hierarchy, and data-flow queries
// over the SemAnno AST.
//
// Methods:
//   getInScopeSymbols  — variables/functions/parameters visible at a node
//   getCallHierarchy   — callers and callees of a function
//   getDependencyGraph — data-flow dependencies of a node (via VariableRef)

#include <string>
#include <vector>
#include <algorithm>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Expression.h"

class ContextAPI {
public:
    struct Symbol {
        std::string name;
        std::string kind;    // "function", "variable", "parameter"
        std::string nodeId;
    };

    struct CallInfo {
        std::string functionId;
        std::string functionName;
        std::vector<std::string> callerIds;   // functions that call this one
        std::vector<std::string> calleeIds;   // functions called by this one
    };

    void setRoot(ASTNode* root) { root_ = root; }

    // ---------------------------------------------------------------
    //  getInScopeSymbols — walk up from nodeId, collecting symbols
    // ---------------------------------------------------------------
    std::vector<Symbol> getInScopeSymbols(const std::string& nodeId) const {
        ASTNode* node = findById(root_, nodeId);
        if (!node) return {};

        std::vector<Symbol> symbols;

        // Walk up the ancestor chain (including node itself)
        const ASTNode* cur = node;
        while (cur) {
            if (cur->conceptType == "Function") {
                auto* fn = static_cast<const Function*>(cur);
                // Parameters
                for (auto* p : fn->getChildren("parameters")) {
                    if (p->conceptType == "Parameter") {
                        auto* param = static_cast<const Parameter*>(p);
                        addSymbolUnique(symbols, {param->name, "parameter", param->id});
                    }
                }
                // Body variables
                for (auto* child : fn->getChildren("body")) {
                    if (child->conceptType == "Variable") {
                        auto* v = static_cast<const Variable*>(child);
                        addSymbolUnique(symbols, {v->name, "variable", v->id});
                    }
                }
            }
            else if (cur->conceptType == "Module") {
                auto* mod = static_cast<const Module*>(cur);
                // Module-level variables
                for (auto* child : mod->getChildren("variables")) {
                    if (child->conceptType == "Variable") {
                        auto* v = static_cast<const Variable*>(child);
                        addSymbolUnique(symbols, {v->name, "variable", v->id});
                    }
                }
                // Module-level functions
                for (auto* child : mod->getChildren("functions")) {
                    if (child->conceptType == "Function") {
                        auto* fn = static_cast<const Function*>(child);
                        addSymbolUnique(symbols, {fn->name, "function", fn->id});
                    }
                }
            }
            cur = cur->parent;
        }

        return symbols;
    }

    // ---------------------------------------------------------------
    //  getCallHierarchy
    // ---------------------------------------------------------------
    CallInfo getCallHierarchy(const std::string& functionId) const {
        CallInfo info;
        ASTNode* fnNode = findById(root_, functionId);
        if (!fnNode || fnNode->conceptType != "Function") return info;

        auto* fn = static_cast<Function*>(fnNode);
        info.functionId   = fn->id;
        info.functionName = fn->name;

        // Collect ALL FunctionCall nodes in the entire tree
        std::vector<ASTNode*> allCalls;
        collectByType(root_, "FunctionCall", allCalls);

        // callerIds: functions that contain a call to fn->name
        // calleeIds: functions called from within fn
        for (auto* callNode : allCalls) {
            auto* fc = static_cast<FunctionCall*>(callNode);

            if (fc->functionName == fn->name) {
                // Someone calls our function — find the enclosing Function
                Function* caller = enclosingFunction(callNode);
                if (caller && caller->id != fn->id) {
                    addUnique(info.callerIds, caller->id);
                }
            }

            // Is this call inside our function?
            if (isDescendantOf(callNode, fnNode)) {
                // Resolve callee function ID by name
                std::string calleeId = resolveFunctionId(fc->functionName);
                if (!calleeId.empty()) {
                    addUnique(info.calleeIds, calleeId);
                }
            }
        }

        return info;
    }

    // ---------------------------------------------------------------
    //  getDependencyGraph — data-flow deps via VariableReference
    // ---------------------------------------------------------------
    std::vector<std::string> getDependencyGraph(const std::string& nodeId) const {
        ASTNode* node = findById(root_, nodeId);
        if (!node) return {};

        // Collect all VariableReference nodes in the subtree
        std::vector<ASTNode*> refs;
        collectByType(node, "VariableReference", refs);

        std::vector<std::string> deps;
        for (auto* refNode : refs) {
            auto* vr = static_cast<VariableReference*>(refNode);
            // Resolve to the declaration node (Variable or Parameter)
            std::string declId = resolveSymbolId(vr->variableName, refNode);
            if (!declId.empty()) {
                addUnique(deps, declId);
            }
        }
        return deps;
    }

private:
    // --- helpers ---

    ASTNode* findById(ASTNode* node, const std::string& id) const {
        if (!node) return nullptr;
        if (node->id == id) return node;
        for (auto* child : node->allChildren()) {
            auto* found = findById(child, id);
            if (found) return found;
        }
        return nullptr;
    }

    static void collectByType(ASTNode* node, const std::string& type,
                               std::vector<ASTNode*>& out) {
        if (!node) return;
        if (node->conceptType == type) out.push_back(node);
        for (auto* child : node->allChildren()) {
            collectByType(child, type, out);
        }
    }

    // Walk up to find the nearest enclosing Function
    static Function* enclosingFunction(ASTNode* node) {
        ASTNode* cur = node->parent;
        while (cur) {
            if (cur->conceptType == "Function")
                return static_cast<Function*>(cur);
            cur = cur->parent;
        }
        return nullptr;
    }

    // Check if child is a descendant of ancestor
    static bool isDescendantOf(const ASTNode* child, const ASTNode* ancestor) {
        const ASTNode* cur = child->parent;
        while (cur) {
            if (cur == ancestor) return true;
            cur = cur->parent;
        }
        return false;
    }

    // Resolve a function name to its node ID (searching module-level functions)
    std::string resolveFunctionId(const std::string& name) const {
        if (!root_) return "";
        // Find the module (root or first module child)
        const ASTNode* mod = root_;
        if (mod->conceptType != "Module") return "";

        for (auto* child : mod->getChildren("functions")) {
            if (child->conceptType == "Function") {
                auto* fn = static_cast<const Function*>(child);
                if (fn->name == name) return fn->id;
            }
        }
        return "";
    }

    // Resolve a variable/parameter name to its declaration ID,
    // searching up the scope chain from refNode.
    std::string resolveSymbolId(const std::string& name,
                                const ASTNode* refNode) const {
        const ASTNode* cur = refNode;
        while (cur) {
            if (cur->conceptType == "Function") {
                auto* fn = static_cast<const Function*>(cur);
                for (auto* p : fn->getChildren("parameters")) {
                    if (p->conceptType == "Parameter" &&
                        static_cast<const Parameter*>(p)->name == name)
                        return p->id;
                }
                for (auto* child : fn->getChildren("body")) {
                    if (child->conceptType == "Variable" &&
                        static_cast<const Variable*>(child)->name == name)
                        return child->id;
                }
            }
            else if (cur->conceptType == "Module") {
                for (auto* child : cur->getChildren("variables")) {
                    if (child->conceptType == "Variable" &&
                        static_cast<const Variable*>(child)->name == name)
                        return child->id;
                }
            }
            cur = cur->parent;
        }
        return "";
    }

    static void addSymbolUnique(std::vector<Symbol>& symbols,
                                const Symbol& sym) {
        for (const auto& s : symbols) {
            if (s.nodeId == sym.nodeId) return;
        }
        symbols.push_back(sym);
    }

    static void addUnique(std::vector<std::string>& vec,
                          const std::string& val) {
        if (std::find(vec.begin(), vec.end(), val) == vec.end())
            vec.push_back(val);
    }

    ASTNode* root_ = nullptr;
};
