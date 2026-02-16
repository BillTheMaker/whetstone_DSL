// Step 339: C++ Parser — Preprocessor, Enum, Namespace (12 tests)
// Tests tree-sitter C++ parsing of preprocessor directives, enums, namespaces, type aliases

#include <cassert>
#include <iostream>
#include <string>
#include "ast/PreprocessorNodes.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/ClassDeclaration.h"
#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
#include "ast/Parser.h"

int main() {
    int passed = 0;

    // Test 1: #include system parsed
    {
        std::string src = "#include <vector>\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        assert(!stmts.empty());
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "IncludeDirective") {
                auto* inc = static_cast<IncludeDirective*>(s);
                assert(inc->isSystem);
                assert(inc->path == "vector");
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 1 PASSED: #include <vector> parsed\n";
        passed++;
    }

    // Test 2: #include local parsed
    {
        std::string src = "#include \"ast/ASTNode.h\"\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "IncludeDirective") {
                auto* inc = static_cast<IncludeDirective*>(s);
                assert(!inc->isSystem);
                assert(inc->path == "ast/ASTNode.h");
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 2 PASSED: #include \"ast/ASTNode.h\" parsed\n";
        passed++;
    }

    // Test 3: #pragma once
    {
        std::string src = "#pragma once\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "PragmaDirective") {
                auto* prag = static_cast<PragmaDirective*>(s);
                assert(prag->directive == "once");
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 3 PASSED: #pragma once parsed\n";
        passed++;
    }

    // Test 4: #define object-like macro
    {
        std::string src = "#define MAX_SIZE 1024\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "MacroDefinition") {
                auto* mac = static_cast<MacroDefinition*>(s);
                assert(mac->name == "MAX_SIZE");
                assert(!mac->isFunctionLike);
                assert(mac->body.find("1024") != std::string::npos);
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 4 PASSED: #define object-like macro\n";
        passed++;
    }

    // Test 5: #define function-like macro
    {
        std::string src = "#define MIN(a, b) ((a) < (b) ? (a) : (b))\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "MacroDefinition") {
                auto* mac = static_cast<MacroDefinition*>(s);
                assert(mac->name == "MIN");
                assert(mac->isFunctionLike);
                assert(mac->parameters.size() >= 2);
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 5 PASSED: #define function-like macro\n";
        passed++;
    }

    // Test 6: enum class with values
    {
        std::string src = "enum class Color { Red = 0, Green = 1, Blue = 2 };\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "EnumDeclaration") {
                auto* e = static_cast<EnumDeclaration*>(s);
                assert(e->name == "Color");
                assert(e->isScoped);
                auto members = e->getChildren("members");
                assert(members.size() == 3);
                assert(static_cast<EnumMember*>(members[0])->name == "Red");
                assert(static_cast<EnumMember*>(members[0])->value == "0");
                assert(static_cast<EnumMember*>(members[2])->name == "Blue");
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 6 PASSED: enum class with values\n";
        passed++;
    }

    // Test 7: plain enum
    {
        std::string src = "enum Flags { A, B, C };\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "EnumDeclaration") {
                auto* e = static_cast<EnumDeclaration*>(s);
                assert(e->name == "Flags");
                assert(!e->isScoped);
                auto members = e->getChildren("members");
                assert(members.size() == 3);
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 7 PASSED: plain enum parsed\n";
        passed++;
    }

    // Test 8: namespace with contents
    {
        std::string src = R"(
namespace whetstone {
    void helper() {}
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "NamespaceDeclaration") {
                auto* ns = static_cast<NamespaceDeclaration*>(s);
                assert(ns->name == "whetstone");
                auto body = ns->getChildren("body");
                assert(!body.empty());
                assert(body[0]->conceptType == "Function");
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 8 PASSED: namespace with contents\n";
        passed++;
    }

    // Test 9: using alias
    {
        std::string src = "using StringVec = std::vector<std::string>;\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "TypeAlias") {
                auto* ta = static_cast<TypeAlias*>(s);
                assert(ta->aliasName == "StringVec");
                assert(ta->isUsing);
                assert(ta->targetType.find("vector") != std::string::npos);
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 9 PASSED: using alias parsed\n";
        passed++;
    }

    // Test 10: typedef
    {
        std::string src = "typedef unsigned int UINT;\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "TypeAlias") {
                auto* ta = static_cast<TypeAlias*>(s);
                assert(ta->aliasName == "UINT");
                assert(!ta->isUsing);
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 10 PASSED: typedef parsed\n";
        passed++;
    }

    // Test 11: Mixed file with preprocessor + namespace + enum + functions
    {
        std::string src = R"(
#pragma once
#include <string>
#define VERSION 1

namespace ast {
    enum class NodeType { Expression, Statement, Declaration };
    void process() {}
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        int pragmaCount = 0, includeCount = 0, macroCount = 0, nsCount = 0;
        for (auto* s : stmts) {
            if (s->conceptType == "PragmaDirective") pragmaCount++;
            if (s->conceptType == "IncludeDirective") includeCount++;
            if (s->conceptType == "MacroDefinition") macroCount++;
            if (s->conceptType == "NamespaceDeclaration") nsCount++;
        }
        assert(pragmaCount >= 1);
        assert(includeCount >= 1);
        assert(macroCount >= 1);
        assert(nsCount >= 1);
        std::cout << "Test 11 PASSED: Mixed file parsed\n";
        passed++;
    }

    // Test 12: Backward compat — existing function parsing still works
    {
        std::string src = R"(
#include <vector>

class Widget : public Base {
public:
    void render() {}
};

void standalone() {}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        auto funcs = mod->getChildren("functions");
        auto stmts = mod->getChildren("statements");
        assert(!classes.empty());
        assert(static_cast<ClassDeclaration*>(classes[0])->name == "Widget");
        assert(!funcs.empty());
        // Should also have the include
        bool hasInclude = false;
        for (auto* s : stmts) {
            if (s->conceptType == "IncludeDirective") hasInclude = true;
        }
        assert(hasInclude);
        std::cout << "Test 12 PASSED: Backward compat\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
