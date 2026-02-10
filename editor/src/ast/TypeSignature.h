#pragma once
#include "ASTNode.h"
#include <vector>

class TypeSignature : public ASTNode {
public:
    std::string name;
    bool variadic = false;
    std::vector<std::string> semanticTags;

    TypeSignature() { conceptType = "TypeSignature"; }
    TypeSignature(const std::string& id, const std::string& name, bool variadic = false)
        : name(name), variadic(variadic) {
        this->id = id;
        this->conceptType = "TypeSignature";
    }
    // children: returnType (1) via setChild("returnType", Type*)
    // children: paramTypes (0..n) via addChild("paramTypes", Type*)
};
