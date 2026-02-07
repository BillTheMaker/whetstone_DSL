#pragma once
#include "ASTNode.h"

class Type : public ASTNode {
public:
    Type() { conceptType = "Type"; }
};

class PrimitiveType : public Type {
public:
    std::string kind;
    PrimitiveType() { conceptType = "PrimitiveType"; }
    PrimitiveType(const std::string& id, const std::string& kind) : kind(kind) {
        this->id = id;
        this->conceptType = "PrimitiveType";
    }
};

class ListType : public Type {
public:
    ListType() { conceptType = "ListType"; }
    // children: elementType (1) via setChild("elementType", ...)
};

class SetType : public Type {
public:
    SetType() { conceptType = "SetType"; }
    // children: elementType (1) via setChild("elementType", ...)
};

class MapType : public Type {
public:
    MapType() { conceptType = "MapType"; }
    // children: keyType (1), valueType (1) via setChild
};

class TupleType : public Type {
public:
    TupleType() { conceptType = "TupleType"; }
    // children: elementTypes (0..n) via addChild("elementTypes", ...)
};

class ArrayType : public Type {
public:
    ArrayType() { conceptType = "ArrayType"; }
    // children: elementType (1) via setChild("elementType", ...)
    // children: size (1) via setChild("size", ...)  [Expression]
};

class OptionalType : public Type {
public:
    OptionalType() { conceptType = "OptionalType"; }
    // children: innerType (1) via setChild("innerType", ...)
};

class CustomType : public Type {
public:
    std::string typeName;
    CustomType() { conceptType = "CustomType"; }
    CustomType(const std::string& id, const std::string& name) : typeName(name) {
        this->id = id;
        this->conceptType = "CustomType";
    }
};
