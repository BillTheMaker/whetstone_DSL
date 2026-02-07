#pragma once
#include <string>

class ASTNode {
public:
    std::string id;
    std::string conceptType;
    ASTNode* parent = nullptr;

    virtual ~ASTNode() = default;
};
