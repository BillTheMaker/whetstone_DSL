#pragma once
// Step 156: Agent annotation assistant
//
// Provides annotation suggestions + validation diagnostics for a scoped region,
// plus helpers to apply suggestions via ASTMutationAPI and record feedback.

#include <string>
#include <vector>
#include <unordered_set>

#include "MemoryStrategyInference.h"
#include "AnnotationValidator.h"
#include "ASTMutationAPI.h"
#include "ast/Annotation.h"

class AgentAnnotationAssistant {
public:
    struct Result {
        std::vector<MemoryStrategyInference::Suggestion> suggestions;
        std::vector<AnnotationValidator::Diagnostic> diagnostics;
        std::string scopeNodeId;
    };

    Result suggest(ASTNode* root, const std::string& nodeId = "") const {
        Result out;
        if (!root) return out;

        ASTNode* scope = root;
        if (!nodeId.empty()) {
            ASTNode* found = findById(root, nodeId);
            if (found) scope = found;
        }
        out.scopeNodeId = scope->id;

        MemoryStrategyInference inf;
        out.suggestions = inf.inferAnnotations(scope);

        AnnotationValidator validator;
        auto allDiags = validator.validate(root);
        if (scope == root) {
            out.diagnostics = std::move(allDiags);
            return out;
        }

        std::unordered_set<std::string> ids;
        collectNodeIds(scope, ids);
        for (const auto& diag : allDiags) {
            if (ids.find(diag.nodeId) != ids.end()) {
                out.diagnostics.push_back(diag);
            }
        }

        return out;
    }

    ASTMutationAPI::MutationResult applySuggestion(
        ASTNode* root,
        const MemoryStrategyInference::Suggestion& suggestion) const {
        ASTMutationAPI::MutationResult res;
        if (!root) {
            res.error = "AST unavailable";
            return res;
        }

        ASTNode* target = findById(root, suggestion.nodeId);
        if (!target) {
            res.error = "Node not found: " + suggestion.nodeId;
            return res;
        }

        Annotation* anno = createAnnotation(suggestion);
        if (!anno) {
            res.error = "Unknown annotation type: " + suggestion.annotationType;
            return res;
        }

        ASTMutationAPI mut;
        mut.setRoot(root);
        res = mut.insertNode(target->id, "annotations", anno);
        if (!res.success) {
            delete anno;
        }
        return res;
    }

    void recordFeedback(const MemoryStrategyInference::Suggestion& suggestion,
                        bool accepted) const {
        MemoryStrategyInference inf;
        inf.recordFeedback(suggestion, accepted);
    }

private:
    static ASTNode* findById(ASTNode* node, const std::string& id) {
        if (!node) return nullptr;
        if (node->id == id) return node;
        for (auto* child : node->allChildren()) {
            if (auto* found = findById(child, id)) return found;
        }
        return nullptr;
    }

    static void collectNodeIds(ASTNode* node,
                               std::unordered_set<std::string>& ids) {
        if (!node) return;
        ids.insert(node->id);
        for (auto* child : node->allChildren()) {
            collectNodeIds(child, ids);
        }
    }

    static std::string makeAnnotationId(const MemoryStrategyInference::Suggestion& s) {
        return "anno_" + s.annotationType + "_" + s.nodeId;
    }

    static Annotation* createAnnotation(
        const MemoryStrategyInference::Suggestion& s) {
        if (s.annotationType == "ReclaimAnnotation") {
            auto* a = new ReclaimAnnotation(makeAnnotationId(s), s.strategy);
            return a;
        }
        if (s.annotationType == "LifetimeAnnotation") {
            auto* a = new LifetimeAnnotation(makeAnnotationId(s), s.strategy);
            return a;
        }
        if (s.annotationType == "DeallocateAnnotation") {
            auto* a = new DeallocateAnnotation(makeAnnotationId(s), s.strategy);
            return a;
        }
        if (s.annotationType == "OwnerAnnotation") {
            auto* a = new OwnerAnnotation(makeAnnotationId(s), s.strategy);
            return a;
        }
        if (s.annotationType == "AllocateAnnotation") {
            auto* a = new AllocateAnnotation(makeAnnotationId(s), s.strategy);
            return a;
        }
        return nullptr;
    }
};
