#pragma once
// Step 248: Compact AST response format
//
// Token-efficient AST serialization for agent consumption.
// Provides compact mode, subtree extraction, and AST diff support.

#include "ast/ASTNode.h"
#include "ast/Serialization.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;

// --- Extract a human-readable name from any AST node ---
inline std::string getNodeName(const ASTNode* node) {
    if (!node) return "";
    const auto& ct = node->conceptType;
    if (ct == "Module")
        return static_cast<const Module*>(node)->name;
    if (ct == "Function")
        return static_cast<const Function*>(node)->name;
    if (ct == "Variable")
        return static_cast<const Variable*>(node)->name;
    if (ct == "Parameter")
        return static_cast<const Parameter*>(node)->name;
    if (ct == "FunctionCall")
        return static_cast<const FunctionCall*>(node)->functionName;
    if (ct == "VariableReference")
        return static_cast<const VariableReference*>(node)->variableName;
    if (ct == "BinaryOperation")
        return static_cast<const BinaryOperation*>(node)->op;
    if (ct == "UnaryOperation")
        return static_cast<const UnaryOperation*>(node)->op;
    if (ct == "StringLiteral")
        return static_cast<const StringLiteral*>(node)->value;
    if (ct == "Import")
        return static_cast<const Import*>(node)->moduleName;
    if (ct == "ExternalModule")
        return static_cast<const ExternalModule*>(node)->name;
    if (ct == "PrimitiveType")
        return static_cast<const PrimitiveType*>(node)->kind;
    if (ct == "CustomType")
        return static_cast<const CustomType*>(node)->typeName;
    if (ct == "MemberAccess")
        return static_cast<const MemberAccess*>(node)->memberName;
    if (ct == "TypeSignature")
        return static_cast<const TypeSignature*>(node)->name;
    if (ct == "ForLoop")
        return static_cast<const ForLoop*>(node)->iteratorName;
    if (ct == "IntegerLiteral")
        return std::to_string(
            static_cast<const IntegerLiteral*>(node)->value);
    if (ct == "BooleanLiteral")
        return static_cast<const BooleanLiteral*>(node)->value
            ? "true" : "false";
    return "";
}

// --- Compact AST serialization ---
// Returns: {id, type, name, line, childCount, children: [child_ids]}
// Uses short keys and omits empty fields for minimal token usage.
inline json toJsonCompact(const ASTNode* node) {
    if (!node) return json();
    json j;
    j["id"] = node->id;
    j["type"] = node->conceptType;
    std::string name = getNodeName(node);
    if (!name.empty()) j["name"] = name;
    if (node->hasSpan()) j["line"] = node->spanStartLine;

    auto kids = node->allChildren();
    if (!kids.empty()) {
        j["childCount"] = (int)kids.size();
        json childIds = json::array();
        for (const auto* child : kids)
            childIds.push_back(child->id);
        j["children"] = childIds;
    }

    return j;
}

// Compact summary: top-level nodes only (Module + direct children).
// Deeper nodes omitted — use getASTSubtree for detail.
inline json toJsonCompactSummary(const ASTNode* root) {
    if (!root) return json::array();
    json nodes = json::array();
    // Root node
    json rootJ;
    rootJ["id"] = root->id;
    rootJ["type"] = root->conceptType;
    std::string rname = getNodeName(root);
    if (!rname.empty()) rootJ["name"] = rname;
    if (root->hasSpan()) rootJ["line"] = root->spanStartLine;
    auto rootKids = root->allChildren();
    if (!rootKids.empty()) rootJ["childCount"] = (int)rootKids.size();
    nodes.push_back(rootJ);
    // Direct children (depth 1 only — functions, imports, etc.)
    for (const auto* child : rootKids) {
        json cj;
        cj["id"] = child->id;
        cj["type"] = child->conceptType;
        std::string cname = getNodeName(child);
        if (!cname.empty()) cj["name"] = cname;
        if (child->hasSpan()) cj["line"] = child->spanStartLine;
        auto grandkids = child->allChildren();
        if (!grandkids.empty())
            cj["childCount"] = (int)grandkids.size();
        nodes.push_back(cj);
    }
    return nodes;
}

// Collect all nodes in compact format (flat list)
inline json toJsonCompactTree(const ASTNode* node) {
    if (!node) return json::array();
    json nodes = json::array();
    nodes.push_back(toJsonCompact(node));
    for (const auto* child : node->allChildren()) {
        json childNodes = toJsonCompactTree(child);
        for (auto& cn : childNodes)
            nodes.push_back(std::move(cn));
    }
    return nodes;
}

// --- Subtree extraction ---
// Returns full JSON for the subtree rooted at nodeId
inline json toJsonSubtree(ASTNode* root, const std::string& nodeId) {
    ASTNode* target = findNodeById(root, nodeId);
    if (!target) return json();
    return toJson(target);
}

// --- Token estimate ---
// Rough estimate: characters / 4 (approximates LLM tokens)
inline int tokenEstimate(const json& j) {
    std::string s = j.dump();
    return (int)s.size() / 4;
}

// --- AST version tracking ---
// Stored in HeadlessBufferState, records which node IDs changed per version.
struct ASTVersionTracker {
    int version = 0;
    // version -> list of affected node IDs
    std::map<int, std::vector<std::string>> changes;

    void recordMutation(const std::vector<std::string>& affectedIds) {
        ++version;
        changes[version] = affectedIds;
    }

    // Get all node IDs that changed since a given version
    std::vector<std::string> changedSince(int sinceVersion) const {
        std::vector<std::string> result;
        for (const auto& [v, ids] : changes) {
            if (v > sinceVersion) {
                for (const auto& id : ids)
                    result.push_back(id);
            }
        }
        return result;
    }

    // Build a diff response: full JSON for changed nodes only
    json buildDiff(ASTNode* root, int sinceVersion) const {
        auto changedIds = changedSince(sinceVersion);
        json nodes = json::array();
        for (const auto& id : changedIds) {
            ASTNode* node = findNodeById(root, id);
            if (node)
                nodes.push_back(toJson(node));
        }
        return {
            {"sinceVersion", sinceVersion},
            {"currentVersion", version},
            {"changedCount", (int)nodes.size()},
            {"nodes", nodes}
        };
    }

    // Prune old entries to prevent unbounded growth
    void pruneOlderThan(int keepVersion) {
        auto it = changes.begin();
        while (it != changes.end() && it->first < keepVersion)
            it = changes.erase(it);
    }
};
