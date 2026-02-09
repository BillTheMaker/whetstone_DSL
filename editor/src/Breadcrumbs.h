#pragma once
// Step 112: Breadcrumb navigation helpers

#include <string>
#include <vector>
#include <algorithm>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"

struct BreadcrumbItem {
    std::string label;
    const ASTNode* node = nullptr;
    bool isRole = false;
};

inline std::string breadcrumbNodeLabel(const ASTNode* node) {
    if (!node) return "";
    if (node->conceptType == "Function") {
        auto* fn = static_cast<const Function*>(node);
        return std::string("Function:") + fn->name;
    }
    if (node->conceptType == "Variable") {
        auto* var = static_cast<const Variable*>(node);
        return std::string("Variable:") + var->name;
    }
    if (node->conceptType == "Parameter") {
        auto* param = static_cast<const Parameter*>(node);
        return std::string("Parameter:") + param->name;
    }
    return node->conceptType;
}

inline std::string breadcrumbRoleLabel(const ASTNode* parent, const ASTNode* child) {
    if (!parent || !child) return "";
    for (const auto& role : parent->childRoles()) {
        const auto& kids = parent->getChildren(role);
        for (auto* k : kids) {
            if (k == child) return role;
        }
    }
    return "";
}

inline std::vector<BreadcrumbItem> buildBreadcrumbTrail(const ASTNode* node) {
    std::vector<const ASTNode*> chain;
    const ASTNode* cur = node;
    while (cur) {
        chain.push_back(cur);
        cur = cur->parent;
    }
    std::reverse(chain.begin(), chain.end());

    std::vector<BreadcrumbItem> items;
    for (size_t i = 0; i < chain.size(); ++i) {
        BreadcrumbItem item;
        item.label = breadcrumbNodeLabel(chain[i]);
        item.node = chain[i];
        items.push_back(std::move(item));

        if (i + 1 < chain.size()) {
            std::string role = breadcrumbRoleLabel(chain[i], chain[i + 1]);
            if (!role.empty()) {
                BreadcrumbItem roleItem;
                roleItem.label = role;
                roleItem.node = nullptr;
                roleItem.isRole = true;
                items.push_back(std::move(roleItem));
            }
        }
    }
    return items;
}
