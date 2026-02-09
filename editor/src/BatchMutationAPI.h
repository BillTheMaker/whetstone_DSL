#pragma once
// Step 63: Batch operations (extend)
//
// BatchMutationAPI: atomic all-or-nothing mutation sequences.
//
// applySequence(mutations) tries each mutation in order.  If any
// mutation fails, every preceding mutation is rolled back in reverse
// order so the AST returns to its pre-batch state.
//
// Supported mutation types: "setProperty", "insertNode", "deleteNode".

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Expression.h"

class BatchMutationAPI {
public:
    struct Mutation {
        std::string type;       // "setProperty", "insertNode", "deleteNode"
        std::string nodeId;
        std::string property;   // for setProperty
        std::string value;      // for setProperty
        std::string parentId;   // for insertNode
        std::string role;       // for insertNode
        ASTNode* newNode = nullptr; // for insertNode
    };

    struct BatchResult {
        bool success = false;
        int appliedCount = 0;
        std::string error;
    };

    void setRoot(ASTNode* root) { root_ = root; }

    BatchResult applySequence(const std::vector<Mutation>& mutations) {
        BatchResult res;
        std::vector<UndoAction> undoStack;

        for (size_t i = 0; i < mutations.size(); ++i) {
            const auto& mut = mutations[i];
            std::string err;

            if (mut.type == "setProperty") {
                err = applySetProperty(mut, undoStack);
            } else if (mut.type == "insertNode") {
                err = applyInsertNode(mut, undoStack);
            } else if (mut.type == "deleteNode") {
                err = applyDeleteNode(mut, undoStack);
            } else {
                err = "Unknown mutation type: " + mut.type;
            }

            if (!err.empty()) {
                // Roll back everything applied so far
                rollback(undoStack);
                res.success = false;
                res.appliedCount = static_cast<int>(i);
                res.error = err;
                return res;
            }
        }

        // All succeeded — record in journal
        for (const auto& undo : undoStack) {
            journal_.push_back(undo.description);
        }

        res.success = true;
        res.appliedCount = static_cast<int>(mutations.size());
        return res;
    }

    std::vector<std::string> getJournalEntries() const { return journal_; }
    void clearJournal() { journal_.clear(); }

private:
    // An undo action captures how to reverse a single mutation.
    struct UndoAction {
        std::function<void()> undo;
        std::string description;
    };

    // --- mutation implementations ---

    std::string applySetProperty(const Mutation& mut,
                                 std::vector<UndoAction>& undoStack) {
        ASTNode* node = findById(root_, mut.nodeId);
        if (!node)
            return "Node not found: " + mut.nodeId;

        // Read old value
        std::string oldValue = getStringProperty(node, mut.property);

        if (!setStringProperty(node, mut.property, mut.value))
            return "Cannot set property '" + mut.property +
                   "' on " + node->conceptType;

        // Capture undo
        std::string nid = mut.nodeId;
        std::string prop = mut.property;
        ASTNode** rootPtr = &root_;
        undoStack.push_back({
            [node, prop, oldValue, this]() {
                setStringProperty(node, prop, oldValue);
            },
            "setProperty " + mut.nodeId + "." + mut.property + " = " + mut.value
        });
        return "";
    }

    std::string applyInsertNode(const Mutation& mut,
                                std::vector<UndoAction>& undoStack) {
        ASTNode* parent = findById(root_, mut.parentId);
        if (!parent)
            return "Parent not found: " + mut.parentId;
        if (!mut.newNode)
            return "Node is null for insertNode";

        parent->addChild(mut.role, mut.newNode);

        ASTNode* inserted = mut.newNode;
        undoStack.push_back({
            [parent, inserted]() {
                parent->removeChild(inserted);
            },
            "insertNode " + inserted->id + " into " + mut.parentId + "." + mut.role
        });
        return "";
    }

    std::string applyDeleteNode(const Mutation& mut,
                                std::vector<UndoAction>& undoStack) {
        ASTNode* node = findById(root_, mut.nodeId);
        if (!node)
            return "Node not found: " + mut.nodeId;
        if (!node->parent)
            return "Cannot delete root node";

        ASTNode* parent = node->parent;
        std::string role = findRole(parent, node);
        if (role.empty())
            return "Could not determine role for node " + mut.nodeId;

        if (!parent->removeChild(node))
            return "Failed to remove node from parent";

        undoStack.push_back({
            [parent, node, role]() {
                parent->addChild(role, node);
            },
            "deleteNode " + mut.nodeId
        });
        return "";
    }

    // --- rollback ---

    static void rollback(std::vector<UndoAction>& undoStack) {
        for (auto it = undoStack.rbegin(); it != undoStack.rend(); ++it) {
            it->undo();
        }
        undoStack.clear();
    }

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

    // Find which role a child occupies in its parent
    static std::string findRole(const ASTNode* parent, const ASTNode* child) {
        for (const auto& role : parent->childRoles()) {
            for (const auto* kid : parent->getChildren(role)) {
                if (kid == child) return role;
            }
        }
        return "";
    }

    static std::string getStringProperty(const ASTNode* node,
                                         const std::string& prop) {
        const auto& ct = node->conceptType;
        if (prop == "name") {
            if (ct == "Function")  return static_cast<const Function*>(node)->name;
            if (ct == "Variable")  return static_cast<const Variable*>(node)->name;
            if (ct == "Parameter") return static_cast<const Parameter*>(node)->name;
            if (ct == "Module")    return static_cast<const Module*>(node)->name;
        }
        if (prop == "op" && ct == "BinaryOperation")
            return static_cast<const BinaryOperation*>(node)->op;
        if (prop == "variableName" && ct == "VariableReference")
            return static_cast<const VariableReference*>(node)->variableName;
        if (prop == "targetLanguage" && ct == "Module")
            return static_cast<const Module*>(node)->targetLanguage;
        return "";
    }

    bool setStringProperty(ASTNode* node, const std::string& prop,
                           const std::string& value) const {
        const auto& ct = node->conceptType;
        if (prop == "name") {
            if (ct == "Function")  { static_cast<Function*>(node)->name  = value; return true; }
            if (ct == "Variable")  { static_cast<Variable*>(node)->name  = value; return true; }
            if (ct == "Parameter") { static_cast<Parameter*>(node)->name = value; return true; }
            if (ct == "Module")    { static_cast<Module*>(node)->name    = value; return true; }
        }
        if (prop == "op" && ct == "BinaryOperation") {
            static_cast<BinaryOperation*>(node)->op = value; return true;
        }
        if (prop == "variableName" && ct == "VariableReference") {
            static_cast<VariableReference*>(node)->variableName = value; return true;
        }
        if (prop == "targetLanguage" && ct == "Module") {
            static_cast<Module*>(node)->targetLanguage = value; return true;
        }
        return false;
    }

    ASTNode* root_ = nullptr;
    std::vector<std::string> journal_;
};
