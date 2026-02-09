#pragma once
// Step 102: Memory strategy dashboard helpers

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "ast/Function.h"
#include "ast/Module.h"

struct AnnotationEntry {
    std::string nodeId;
    std::string nodeName;
    std::string label;
    int line = -1;
};

inline bool isMemoryAnnotation(const ASTNode* anno) {
    if (!anno) return false;
    const auto& ct = anno->conceptType;
    return ct == "DeallocateAnnotation" || ct == "LifetimeAnnotation" ||
           ct == "ReclaimAnnotation" || ct == "OwnerAnnotation" ||
           ct == "AllocateAnnotation";
}

inline std::string annotationLabelFor(const ASTNode* anno) {
    if (!anno) return "";
    if (anno->conceptType == "ReclaimAnnotation") {
        auto* a = static_cast<const ReclaimAnnotation*>(anno);
        return "@Reclaim(" + a->strategy + ")";
    }
    if (anno->conceptType == "OwnerAnnotation") {
        auto* a = static_cast<const OwnerAnnotation*>(anno);
        return "@Owner(" + a->strategy + ")";
    }
    if (anno->conceptType == "DeallocateAnnotation") {
        auto* a = static_cast<const DeallocateAnnotation*>(anno);
        return "@Deallocate(" + a->strategy + ")";
    }
    if (anno->conceptType == "LifetimeAnnotation") {
        auto* a = static_cast<const LifetimeAnnotation*>(anno);
        return "@Lifetime(" + a->strategy + ")";
    }
    if (anno->conceptType == "AllocateAnnotation") {
        auto* a = static_cast<const AllocateAnnotation*>(anno);
        return "@Allocate(" + a->strategy + ")";
    }
    return anno->conceptType;
}

inline std::string nodeNameFor(const ASTNode* node) {
    if (!node) return "";
    if (node->conceptType == "Function") return static_cast<const Function*>(node)->name;
    if (node->conceptType == "Module") return static_cast<const Module*>(node)->name;
    return node->conceptType;
}

inline void collectAnnotationEntries(const ASTNode* node, std::vector<AnnotationEntry>& out) {
    if (!node) return;
    for (auto* anno : node->getChildren("annotations")) {
        if (!isMemoryAnnotation(anno)) continue;
        AnnotationEntry e;
        e.nodeId = node->id;
        e.nodeName = nodeNameFor(node);
        e.label = annotationLabelFor(anno);
        if (node->hasSpan()) e.line = node->spanStartLine;
        out.push_back(std::move(e));
    }
    for (auto* child : node->allChildren()) {
        collectAnnotationEntries(child, out);
    }
}

inline nlohmann::json buildAnnotationSummaryJson(const std::vector<AnnotationEntry>& entries) {
    nlohmann::json j;
    j["annotations"] = nlohmann::json::array();
    for (const auto& e : entries) {
        j["annotations"].push_back({
            {"nodeId", e.nodeId},
            {"nodeName", e.nodeName},
            {"label", e.label},
            {"line", e.line}
        });
    }
    return j;
}
