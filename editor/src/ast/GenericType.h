#pragma once
#include "ASTNode.h"
#include <string>

// GenericType — a parameterized type like List<T> or Map<K, V>
class GenericType : public ASTNode {
public:
    std::string baseName;  // e.g. "List", "Map", "Optional"

    GenericType() { conceptType = "GenericType"; }
    GenericType(const std::string& id_, const std::string& base)
        : baseName(base) {
        this->id = id_;
        this->conceptType = "GenericType";
    }
    // Children roles: "typeParameters" (list of TypeParameter or PrimitiveType)
};

// TypeParameter — a type variable like T, K, V with optional constraint
class TypeParameter : public ASTNode {
public:
    std::string name;       // e.g. "T", "K"
    std::string constraint; // e.g. "Comparable", empty if unconstrained

    TypeParameter() { conceptType = "TypeParameter"; }
    TypeParameter(const std::string& id_, const std::string& name_)
        : name(name_) {
        this->id = id_;
        this->conceptType = "TypeParameter";
    }
};
