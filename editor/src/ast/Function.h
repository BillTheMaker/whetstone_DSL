#pragma once
#include "ASTNode.h"

class Function : public ASTNode {
public:
    std::string name;

    Function() { conceptType = "Function"; }
    Function(const std::string& id, const std::string& name)
        : name(name) {
        this->id = id;
        this->conceptType = "Function";
    }
};
