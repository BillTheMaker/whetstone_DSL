#pragma once
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
#include "ast/Expression.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

struct LibraryPolicyResult {
    bool ok = true;
    std::string warning;
    std::string error;
    std::vector<std::string> unknownFunctions;
};

static inline void collectFunctionCalls(ASTNode* node, std::vector<std::string>& out) {
    if (!node) return;
    if (node->conceptType == "FunctionCall") {
        auto* call = static_cast<FunctionCall*>(node);
        if (!call->functionName.empty()) out.push_back(call->functionName);
    }
    for (auto* child : node->allChildren()) {
        collectFunctionCalls(child, out);
    }
}

static inline bool matchesAllowed(const std::unordered_set<std::string>& allowed,
                                  const std::string& name) {
    if (allowed.count(name) > 0) return true;
    for (const auto& item : allowed) {
        if (item.size() > name.size()) {
            if (item.rfind("." + name) == item.size() - name.size() - 1) return true;
            if (item.rfind("::" + name) == item.size() - name.size() - 2) return true;
        }
    }
    return false;
}

static inline std::vector<std::string> collectAllowedFunctions(Module* ast) {
    std::vector<std::string> out;
    if (!ast) return out;
    for (auto* fnNode : ast->getChildren("functions")) {
        if (fnNode->conceptType != "Function") continue;
        auto* fn = static_cast<Function*>(fnNode);
        if (!fn->name.empty()) out.push_back(fn->name);
    }
    for (auto* extNode : ast->getChildren("externalModules")) {
        if (extNode->conceptType != "ExternalModule") continue;
        auto* ext = static_cast<ExternalModule*>(extNode);
        for (auto* sigNode : ext->getChildren("signatures")) {
            if (sigNode->conceptType != "TypeSignature") continue;
            auto* sig = static_cast<TypeSignature*>(sigNode);
            if (!sig->name.empty()) out.push_back(sig->name);
        }
    }
    return out;
}

static inline LibraryPolicyResult checkMutationLibraryPolicy(ASTNode* node,
                                                             Module* ast,
                                                             bool preferImports,
                                                             bool strictMode) {
    LibraryPolicyResult res;
    if (!preferImports) return res;
    auto allowedList = collectAllowedFunctions(ast);
    std::unordered_set<std::string> allowed(allowedList.begin(), allowedList.end());

    std::vector<std::string> calls;
    collectFunctionCalls(node, calls);
    std::vector<std::string> unknown;
    for (const auto& name : calls) {
        if (!matchesAllowed(allowed, name)) {
            if (std::find(unknown.begin(), unknown.end(), name) == unknown.end()) {
                unknown.push_back(name);
            }
        }
    }

    if (!unknown.empty()) {
        res.unknownFunctions = unknown;
        std::string alt;
        for (size_t i = 0; i < std::min<size_t>(allowedList.size(), 5); ++i) {
            if (!alt.empty()) alt += ", ";
            alt += allowedList[i];
        }
        std::string message = "References functions outside imports: ";
        for (size_t i = 0; i < unknown.size(); ++i) {
            if (i > 0) message += ", ";
            message += unknown[i];
        }
        if (!alt.empty()) message += " | available: " + alt;
        if (strictMode) {
            res.ok = false;
            res.error = message;
        } else {
            res.warning = message;
        }
    }

    return res;
}
