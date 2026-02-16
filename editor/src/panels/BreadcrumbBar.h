#pragma once
// Step 356: Breadcrumb bar model for AST navigation.
// Headless model used for unit/integration testing and UI adapter layers.

#include <algorithm>
#include <string>
#include <vector>
#include "../ThemeData.h"
#include "../CompactAST.h"
#include "../ast/ASTNode.h"
#include "../ast/Module.h"

struct BreadcrumbSegment {
    std::string label;
    const ASTNode* node = nullptr;
    std::vector<const ASTNode*> siblings;
};

struct BreadcrumbBarModel {
    std::vector<BreadcrumbSegment> segments;
    bool overflow = false;
    Color4 bgColor;
    Color4 textColor;
    Color4 borderColor;
};

inline std::string breadcrumbDisplayName(const ASTNode* node) {
    if (!node) return "";
    std::string name = getNodeName(node);
    if (!name.empty()) return name;
    return node->conceptType;
}

inline std::vector<const ASTNode*> breadcrumbSiblings(const ASTNode* node) {
    if (!node || !node->parent) return {};
    const ASTNode* parent = node->parent;
    for (const auto& role : parent->childRoles()) {
        const auto& kids = parent->getChildren(role);
        bool found = false;
        for (auto* kid : kids) {
            if (kid == node) {
                found = true;
                break;
            }
        }
        if (found) {
            std::vector<const ASTNode*> siblings;
            for (auto* kid : kids) siblings.push_back(kid);
            return siblings;
        }
    }
    return {};
}

inline BreadcrumbBarModel buildBreadcrumbBar(const Module* module,
                                             const ASTNode* cursorNode,
                                             const WhetstoneTheme& theme,
                                             size_t maxSegments = 8) {
    BreadcrumbBarModel model;
    model.bgColor = theme.bgPanel;
    model.textColor = theme.text;
    model.borderColor = theme.border;

    if (!module) return model;

    std::vector<const ASTNode*> chain;
    if (!cursorNode) {
        chain.push_back(module);
    } else {
        const ASTNode* cur = cursorNode;
        while (cur) {
            chain.push_back(cur);
            if (cur == module) break;
            cur = cur->parent;
        }
        if (chain.empty() || chain.back() != module) {
            chain.clear();
            chain.push_back(module);
        } else {
            std::reverse(chain.begin(), chain.end());
        }
    }

    for (auto* node : chain) {
        BreadcrumbSegment seg;
        seg.node = node;
        seg.label = breadcrumbDisplayName(node);
        seg.siblings = breadcrumbSiblings(node);
        model.segments.push_back(seg);
    }

    model.overflow = model.segments.size() > maxSegments;
    return model;
}

inline const ASTNode* navigateBreadcrumbClick(const BreadcrumbBarModel& model, size_t index) {
    if (index >= model.segments.size()) return nullptr;
    return model.segments[index].node;
}

inline std::vector<std::string> breadcrumbSiblingNames(const BreadcrumbSegment& segment) {
    std::vector<std::string> names;
    for (auto* sib : segment.siblings) {
        names.push_back(breadcrumbDisplayName(sib));
    }
    return names;
}
