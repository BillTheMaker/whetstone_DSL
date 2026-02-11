#pragma once
// ASTUtils.h — Pure AST utility functions (no ImGui/SDL dependency).
// Extracted for use by both EditorUtils.h and HeadlessEditorState.h.

#include "ast/ASTNode.h"
#include "ast/Serialization.h"
#include <memory>
#include <string>

// Clone a Module tree (deep copy via JSON roundtrip)
static inline std::unique_ptr<Module> cloneModule(const Module* ast) {
    if (!ast) return nullptr;
    json j = toJson(ast);
    ASTNode* node = fromJson(j);
    if (!node || node->conceptType != "Module") return nullptr;
    return std::unique_ptr<Module>(static_cast<Module*>(node));
}

// Check if a node is an annotation type
static inline bool isAnnotationNode(const ASTNode* node) {
    if (!node) return false;
    if (node->conceptType.find("Annotation") != std::string::npos) return true;
    if (node->conceptType == "DerefStrategy") return true;
    if (node->conceptType == "OptimizationLock") return true;
    if (node->conceptType == "LangSpecific") return true;
    return false;
}

// Count annotation nodes in an AST subtree
static inline int countAnnotationNodes(const ASTNode* node) {
    if (!node) return 0;
    int count = isAnnotationNode(node) ? 1 : 0;
    for (const auto* child : node->allChildren())
        count += countAnnotationNodes(child);
    return count;
}

// Find a node by ID in an AST tree
static inline ASTNode* findNodeById(ASTNode* node,
                                     const std::string& id) {
    if (!node) return nullptr;
    if (node->id == id) return node;
    for (auto* child : node->allChildren())
        if (auto* found = findNodeById(child, id)) return found;
    return nullptr;
}

// Check if a span contains a position
static inline bool spanContains(const ASTNode* node, int line, int col) {
    if (!node || !node->hasSpan()) return false;
    if (line < node->spanStartLine || line > node->spanEndLine) return false;
    if (line == node->spanStartLine && col < node->spanStartCol) return false;
    if (line == node->spanEndLine && col > node->spanEndCol) return false;
    return true;
}

// Span score (smaller span = better match)
static inline int spanScore(const ASTNode* node) {
    if (!node || !node->hasSpan()) return 999999;
    return (node->spanEndLine - node->spanStartLine) * 1000 +
           (node->spanEndCol - node->spanStartCol);
}

// Find a node at a line/col position
static inline ASTNode* findNodeAtPosition(ASTNode* node,
                                           int line, int col) {
    if (!node) return nullptr;
    ASTNode* best = nullptr;
    if (spanContains(node, line, col)) best = node;
    for (auto* child : node->allChildren()) {
        ASTNode* cand = findNodeAtPosition(child, line, col);
        if (cand) {
            if (!best || spanScore(cand) < spanScore(best)) best = cand;
        }
    }
    return best;
}
