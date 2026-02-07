#pragma once
#include "ASTNode.h"

class Variable : public ASTNode {
public:
    std::string name;

    Variable() { conceptType = "Variable"; }
    Variable(const std::string& id, const std::string& name)
        : name(name) {
        this->id = id;
        this->conceptType = "Variable";
    }
};
