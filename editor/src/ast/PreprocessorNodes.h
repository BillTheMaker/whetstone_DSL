#pragma once
#include "ASTNode.h"
#include <string>
#include <vector>

// Preprocessor directive AST nodes — statement-level, added to Module children.
// Preprocessor operates on text, not AST; macro bodies stay as unparsed text.

class IncludeDirective : public ASTNode {
public:
    std::string path;       // "ast/ASTNode.h" or "string"
    bool isSystem = false;  // angle brackets (true) vs quotes (false)

    IncludeDirective() { conceptType = "IncludeDirective"; }
    IncludeDirective(const std::string& id, const std::string& p, bool sys = false)
        : path(p), isSystem(sys) {
        this->id = id;
        conceptType = "IncludeDirective";
    }
};

class PragmaDirective : public ASTNode {
public:
    std::string directive;  // "once", "pack(push, 1)", etc.

    PragmaDirective() { conceptType = "PragmaDirective"; }
    PragmaDirective(const std::string& id, const std::string& dir)
        : directive(dir) {
        this->id = id;
        conceptType = "PragmaDirective";
    }
};

class MacroDefinition : public ASTNode {
public:
    std::string name;                    // Macro name
    std::vector<std::string> parameters; // Empty for object-like macros
    std::string body;                    // Unparsed macro body text
    bool isFunctionLike = false;         // Has parameter list

    MacroDefinition() { conceptType = "MacroDefinition"; }
    MacroDefinition(const std::string& id, const std::string& n)
        : name(n) {
        this->id = id;
        conceptType = "MacroDefinition";
    }
};
