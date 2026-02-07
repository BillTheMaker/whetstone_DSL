#pragma once
#include <string>
#include <vector>
#include <map>

class ASTNode {
public:
    std::string id;
    std::string conceptType;
    ASTNode* parent = nullptr;

    virtual ~ASTNode() = default;

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

    // Get all children across all roles
    std::vector<ASTNode*> allChildren() const {
        std::vector<ASTNode*> result;
        for (const auto& [role, kids] : children_) {
            result.insert(result.end(), kids.begin(), kids.end());
        }
        return result;
    }

private:
    std::map<std::string, std::vector<ASTNode*>> children_;
};
