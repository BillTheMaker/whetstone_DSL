#pragma once
// Step 61: AST mutation API (extend)
//
// ASTMutationAPI: provides validated AST mutations with journal recording
// and OptimizationLock awareness.
//
// Methods:
//   setProperty   — set a single property on a node
//   updateNode    — bulk property update
//   deleteNode    — remove a node from its parent
//   insertNode    — add a node as child of a parent in a given role
//
// All mutations check for OptimizationLock annotations on the target
// node or its ancestors.  If a lock is found the mutation still succeeds
// but a warning is returned (per spec: "warn, don't reject").

#include <string>
#include <vector>
#include <map>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"

class ASTMutationAPI {
public:
    struct MutationResult {
        bool success = false;
        std::string warning;   // non-empty if a warning was produced
        std::string error;     // non-empty if the mutation failed
    };

    void setRoot(ASTNode* root) { root_ = root; }

    // --- mutations ---

    MutationResult setProperty(const std::string& nodeId,
                               const std::string& property,
                               const std::string& value) {
        MutationResult res;
        ASTNode* node = findById(root_, nodeId);
        if (!node) {
            res.error = "Node not found: " + nodeId;
            return res;
        }

        // Lock check
        std::string lockWarning = checkLock(node);

        if (!applyProperty(node, property, value)) {
            res.error = "Cannot set property '" + property +
                        "' on " + node->conceptType;
            return res;
        }

        // Record in journal
        journal_.push_back("setProperty " + nodeId + "." + property +
                           " = " + value);

        res.success = true;
        res.warning = lockWarning;
        return res;
    }

    MutationResult updateNode(const std::string& nodeId,
                              const std::map<std::string, std::string>& properties) {
        MutationResult res;
        ASTNode* node = findById(root_, nodeId);
        if (!node) {
            res.error = "Node not found: " + nodeId;
            return res;
        }

        std::string lockWarning = checkLock(node);

        for (const auto& [prop, val] : properties) {
            if (!applyProperty(node, prop, val)) {
                res.error = "Cannot set property '" + prop +
                            "' on " + node->conceptType;
                return res;
            }
        }

        journal_.push_back("updateNode " + nodeId + " (" +
                           std::to_string(properties.size()) + " properties)");

        res.success = true;
        res.warning = lockWarning;
        return res;
    }

    MutationResult deleteNode(const std::string& nodeId) {
        MutationResult res;
        ASTNode* node = findById(root_, nodeId);
        if (!node) {
            res.error = "Node not found: " + nodeId;
            return res;
        }
        if (!node->parent) {
            res.error = "Cannot delete root node";
            return res;
        }

        std::string lockWarning = checkLock(node);

        ASTNode* par = node->parent;
        if (!par->removeChild(node)) {
            res.error = "Failed to remove node from parent";
            return res;
        }

        journal_.push_back("deleteNode " + nodeId);

        res.success = true;
        res.warning = lockWarning;
        return res;
    }

    MutationResult insertNode(const std::string& parentId,
                              const std::string& role,
                              ASTNode* node) {
        MutationResult res;
        ASTNode* parent = findById(root_, parentId);
        if (!parent) {
            res.error = "Parent not found: " + parentId;
            return res;
        }
        if (!node) {
            res.error = "Node is null";
            return res;
        }

        std::string lockWarning = checkLock(parent);

        parent->addChild(role, node);

        journal_.push_back("insertNode " + node->id + " into " +
                           parentId + "." + role);

        res.success = true;
        res.warning = lockWarning;
        return res;
    }

    // --- journal ---

    std::vector<std::string> getJournalEntries() const { return journal_; }
    void clearJournal() { journal_.clear(); }

private:
    // Recursive node lookup
    ASTNode* findById(ASTNode* node, const std::string& nodeId) const {
        if (!node) return nullptr;
        if (node->id == nodeId) return node;
        for (auto* child : node->allChildren()) {
            auto* found = findById(child, nodeId);
            if (found) return found;
        }
        return nullptr;
    }

    // Walk up the ancestor chain looking for an OptimizationLock annotation.
    // Returns a warning string if locked, empty string otherwise.
    std::string checkLock(const ASTNode* node) const {
        const ASTNode* cur = node;
        while (cur) {
            for (const auto* anno : cur->getChildren("annotations")) {
                if (anno->conceptType == "OptimizationLock") {
                    auto* lock = static_cast<const OptimizationLock*>(anno);
                    return "Node is under OptimizationLock (locked by " +
                           lock->lockedBy + ": " + lock->lockReason + ")";
                }
            }
            cur = cur->parent;
        }
        return "";
    }

    // Apply a string property to a node.  Returns false if the property
    // is not recognised for the node's concept type.
    bool applyProperty(ASTNode* node, const std::string& property,
                       const std::string& value) const {
        const auto& ct = node->conceptType;
        if (property == "name") {
            if (ct == "Function")  { static_cast<Function*>(node)->name  = value; return true; }
            if (ct == "Variable")  { static_cast<Variable*>(node)->name  = value; return true; }
            if (ct == "Parameter") { static_cast<Parameter*>(node)->name = value; return true; }
            if (ct == "Module")    { static_cast<Module*>(node)->name    = value; return true; }
        }
        if (property == "strategy") {
            if (ct == "DerefStrategy")        { static_cast<DerefStrategy*>(node)->strategy        = value; return true; }
            if (ct == "DeallocateAnnotation") { static_cast<DeallocateAnnotation*>(node)->strategy = value; return true; }
            if (ct == "LifetimeAnnotation")   { static_cast<LifetimeAnnotation*>(node)->strategy   = value; return true; }
            if (ct == "ReclaimAnnotation")    { static_cast<ReclaimAnnotation*>(node)->strategy    = value; return true; }
            if (ct == "OwnerAnnotation")      { static_cast<OwnerAnnotation*>(node)->strategy      = value; return true; }
            if (ct == "AllocateAnnotation")   { static_cast<AllocateAnnotation*>(node)->strategy   = value; return true; }
        }
        if (property == "op") {
            if (ct == "BinaryOperation") { static_cast<BinaryOperation*>(node)->op = value; return true; }
        }
        if (property == "variableName") {
            if (ct == "VariableReference") { static_cast<VariableReference*>(node)->variableName = value; return true; }
        }
        if (property == "targetLanguage") {
            if (ct == "Module") { static_cast<Module*>(node)->targetLanguage = value; return true; }
        }
        return false;
    }

    ASTNode* root_ = nullptr;
    std::vector<std::string> journal_;
};
