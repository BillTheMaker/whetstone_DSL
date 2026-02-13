#pragma once
// Step 101: Annotation conflict detection helpers

#include <string>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Annotation.h"

struct AnnotationConflict {
    std::string childAnnoId;
    std::string parentAnnoId;
    std::string conceptType;
    std::string parentStrategy;
    std::string message;
    int childLine = -1;
    int parentLine = -1;
};

inline std::string annotationStrategy(const ASTNode* anno) {
    if (!anno) return "";
    if (anno->conceptType == "DeallocateAnnotation")
        return static_cast<const DeallocateAnnotation*>(anno)->strategy;
    if (anno->conceptType == "LifetimeAnnotation")
        return static_cast<const LifetimeAnnotation*>(anno)->strategy;
    if (anno->conceptType == "ReclaimAnnotation")
        return static_cast<const ReclaimAnnotation*>(anno)->strategy;
    if (anno->conceptType == "OwnerAnnotation")
        return static_cast<const OwnerAnnotation*>(anno)->strategy;
    if (anno->conceptType == "AllocateAnnotation")
        return static_cast<const AllocateAnnotation*>(anno)->strategy;
    return "";
}

inline int nodeLineOrFallback(const ASTNode* node, const ASTNode* fallback) {
    if (node && node->hasSpan()) return node->spanStartLine;
    if (fallback && fallback->hasSpan()) return fallback->spanStartLine;
    return -1;
}

inline void collectAnnotationConflicts(const ASTNode* node,
                                       std::vector<AnnotationConflict>& out) {
    if (!node) return;
    for (auto* anno : node->getChildren("annotations")) {
        std::string strategy = annotationStrategy(anno);
        if (strategy.empty()) continue;
        const ASTNode* cur = node->parent;
        while (cur) {
            for (auto* parentAnno : cur->getChildren("annotations")) {
                if (parentAnno->conceptType == anno->conceptType) {
                    std::string parentStrategy = annotationStrategy(parentAnno);
                    if (!parentStrategy.empty() && parentStrategy != strategy) {
                        AnnotationConflict c;
                        c.childAnnoId = anno->id;
                        c.parentAnnoId = parentAnno->id;
                        c.conceptType = anno->conceptType;
                        c.parentStrategy = parentStrategy;
                        c.childLine = nodeLineOrFallback(anno, node);
                        c.parentLine = nodeLineOrFallback(parentAnno, cur);
                        c.message = "Annotation conflict: " + anno->conceptType +
                                    "(" + strategy + ") vs " +
                                    parentAnno->conceptType + "(" + parentStrategy + ")";
                        out.push_back(std::move(c));
                        return;
                    }
                }
            }
            cur = cur->parent;
        }
    }
    for (auto* child : node->allChildren()) {
        collectAnnotationConflicts(child, out);
    }
}

#include "AnnotationConflictExtended.h"
