#pragma once
#include "ASTNode.h"
#include <string>

// Enum, namespace, and type alias AST nodes — statement-level.

class EnumMember : public ASTNode {
public:
    std::string name;
    std::string value;  // Optional explicit value ("1", "0xFF")

    EnumMember() { conceptType = "EnumMember"; }
    EnumMember(const std::string& id, const std::string& n, const std::string& v = "")
        : name(n), value(v) {
        this->id = id;
        conceptType = "EnumMember";
    }
};

class EnumDeclaration : public ASTNode {
public:
    std::string name;
    bool isScoped = false;           // enum class (true) vs plain enum (false)
    std::string underlyingType;      // Optional: "int", "uint8_t", etc.
    // Children role "members": vector of EnumMember nodes

    EnumDeclaration() { conceptType = "EnumDeclaration"; }
    EnumDeclaration(const std::string& id, const std::string& n, bool scoped = false)
        : name(n), isScoped(scoped) {
        this->id = id;
        conceptType = "EnumDeclaration";
    }
};

class NamespaceDeclaration : public ASTNode {
public:
    std::string name;  // Empty for anonymous namespaces
    // Children role "body": statements/declarations inside

    NamespaceDeclaration() { conceptType = "NamespaceDeclaration"; }
    NamespaceDeclaration(const std::string& id, const std::string& n)
        : name(n) {
        this->id = id;
        conceptType = "NamespaceDeclaration";
    }
};

class TypeAlias : public ASTNode {
public:
    std::string aliasName;   // The new name
    std::string targetType;  // What it aliases
    bool isUsing = true;     // using (true) vs typedef (false)

    TypeAlias() { conceptType = "TypeAlias"; }
    TypeAlias(const std::string& id, const std::string& alias, const std::string& target, bool usingStyle = true)
        : aliasName(alias), targetType(target), isUsing(usingStyle) {
        this->id = id;
        conceptType = "TypeAlias";
    }
};
