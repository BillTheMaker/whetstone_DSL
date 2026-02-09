#pragma once
#include "ASTNode.h"

class Import : public ASTNode {
public:
    std::string moduleName;
    std::string alias;
    std::string importKind; // "module", "include", "require"

    Import() { conceptType = "Import"; }
    Import(const std::string& id, const std::string& module,
           const std::string& kind = "module",
           const std::string& alias = "")
        : moduleName(module), alias(alias), importKind(kind) {
        this->id = id;
        this->conceptType = "Import";
    }
};
