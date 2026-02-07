#pragma once
#include "ASTNode.h"

class Parameter : public ASTNode {
public:
    std::string name;
    Parameter() { conceptType = "Parameter"; }
    Parameter(const std::string& id, const std::string& name) : name(name) {
        this->id = id;
        this->conceptType = "Parameter";
    }
    // children: type (1) via setChild("type", ...)
    // children: defaultValue (0..1) via setChild("defaultValue", ...)
};
