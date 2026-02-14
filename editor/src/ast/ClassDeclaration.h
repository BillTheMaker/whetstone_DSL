#pragma once
#include "ASTNode.h"
#include "Function.h"
#include <string>
#include <vector>

// ClassDeclaration — represents a class/struct definition
class ClassDeclaration : public ASTNode {
public:
    std::string name;
    std::string superClass;     // base class name, empty if none
    bool isAbstract = false;

    ClassDeclaration() { conceptType = "ClassDeclaration"; }
    ClassDeclaration(const std::string& id_, const std::string& name_)
        : name(name_) {
        this->id = id_;
        this->conceptType = "ClassDeclaration";
    }
    // Children roles: "interfaces" (list of CustomType), "fields" (list of Variable),
    //                 "methods" (list of MethodDeclaration), "annotations"
};

// InterfaceDeclaration — represents an interface/trait/protocol
class InterfaceDeclaration : public ASTNode {
public:
    std::string name;

    InterfaceDeclaration() { conceptType = "InterfaceDeclaration"; }
    InterfaceDeclaration(const std::string& id_, const std::string& name_)
        : name(name_) {
        this->id = id_;
        this->conceptType = "InterfaceDeclaration";
    }
    // Children roles: "methods" (list of MethodDeclaration), "annotations"
};

// MethodDeclaration — extends Function with class-specific fields
class MethodDeclaration : public Function {
public:
    std::string className;
    bool isStatic = false;
    std::string visibility = "public";  // "public", "private", "protected"
    bool isOverride = false;
    bool isVirtual = false;

    MethodDeclaration() { conceptType = "MethodDeclaration"; }
    MethodDeclaration(const std::string& id_, const std::string& name_)
        : Function(id_, name_) {
        this->conceptType = "MethodDeclaration";
    }
    // Inherits children: "parameters", "body", "returnType", "annotations"
};
