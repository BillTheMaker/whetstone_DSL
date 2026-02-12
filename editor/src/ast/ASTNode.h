#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <map>

class ASTNode {
public:
    std::string id;
    std::string conceptType;
    ASTNode* parent = nullptr;
    int spanStartLine = -1;
    int spanStartCol = -1;
    int spanEndLine = -1;
    int spanEndCol = -1;

    virtual ~ASTNode() = default;

    void setSpan(int startLine, int startCol, int endLine, int endCol) {
        spanStartLine = startLine;
        spanStartCol = startCol;
        spanEndLine = endLine;
        spanEndCol = endCol;
    }

    bool hasSpan() const {
        return spanStartLine >= 0 && spanStartCol >= 0 && spanEndLine >= 0 && spanEndCol >= 0;
    }

    // Multi-valued child: append to role
    void addChild(const std::string& role, ASTNode* child) {
        child->parent = this;
        children_[role].push_back(child);
    }

    // Single-valued child: set (replaces if exists)
    void setChild(const std::string& role, ASTNode* child) {
        auto& vec = children_[role];
        if (!vec.empty()) {
            vec[0]->parent = nullptr;
            vec[0] = child;
        } else {
            vec.push_back(child);
        }
        child->parent = this;
    }

    // Get single-valued child (nullptr if not set)
    ASTNode* getChild(const std::string& role) const {
        auto it = children_.find(role);
        if (it != children_.end() && !it->second.empty()) {
            return it->second[0];
        }
        return nullptr;
    }

    // Get multi-valued children (empty vector if role not present)
    const std::vector<ASTNode*>& getChildren(const std::string& role) const {
        static const std::vector<ASTNode*> empty;
        auto it = children_.find(role);
        return it != children_.end() ? it->second : empty;
    }

    // Get all child role names
    std::vector<std::string> childRoles() const {
        std::vector<std::string> roles;
        for (const auto& [role, _] : children_) {
            roles.push_back(role);
        }
        return roles;
    }

    // Get all children across all roles
    std::vector<ASTNode*> allChildren() const {
        std::vector<ASTNode*> result;
        for (const auto& [role, kids] : children_) {
            result.insert(result.end(), kids.begin(), kids.end());
        }
        return result;
    }

    // Remove a child from whatever role it belongs to
    bool removeChild(ASTNode* child) {
        for (auto& [role, kids] : children_) {
            auto it = std::find(kids.begin(), kids.end(), child);
            if (it != kids.end()) {
                (*it)->parent = nullptr;
                kids.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    std::map<std::string, std::vector<ASTNode*>> children_;
};
