// Step 361: C Parser — Functions, Structs, Enums (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "Pipeline.h"
#include "ast/Parser.h"
#include "ast/ClassDeclaration.h"
#include "ast/EnumNamespaceNodes.h"

int main() {
    int passed = 0;

    // Test 1: function parsing
    {
        auto mod = CParser::parseC("int add(int x, int y) { return x + y; }\n");
        auto fns = mod->getChildren("functions");
        assert(fns.size() == 1);
        auto* fn = static_cast<Function*>(fns[0]);
        assert(fn->name == "add");
        assert(fn->getChildren("parameters").size() == 2);
        std::cout << "Test 1 PASSED: function parsing\n";
        passed++;
    }

    // Test 2: struct parsing
    {
        auto mod = CParser::parseC("struct Point { int x; int y; };\n");
        auto classes = mod->getChildren("classes");
        assert(classes.size() == 1);
        auto* cls = static_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "Point");
        assert(cls->getChildren("fields").size() == 2);
        std::cout << "Test 2 PASSED: struct parsing\n";
        passed++;
    }

    // Test 3: typedef struct parsing
    {
        auto mod = CParser::parseC("typedef struct { int id; char* name; } User;\n");
        auto classes = mod->getChildren("classes");
        assert(classes.size() == 1);
        auto* cls = static_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "User");
        bool foundAlias = false;
        for (auto* s : mod->getChildren("statements")) {
            if (s->conceptType != "TypeAlias") continue;
            auto* ta = static_cast<TypeAlias*>(s);
            if (ta->aliasName == "User" && !ta->isUsing) foundAlias = true;
        }
        assert(foundAlias);
        std::cout << "Test 3 PASSED: typedef struct parsing\n";
        passed++;
    }

    // Test 4: enum parsing
    {
        std::string src = "enum Color { RED=1, GREEN=2, BLUE=3 }; enum { A=1, B=2 };";
        auto mod = CParser::parseC(src);
        int enums = 0;
        bool named = false, anon = false;
        for (auto* s : mod->getChildren("statements")) {
            if (s->conceptType != "EnumDeclaration") continue;
            enums++;
            auto* e = static_cast<EnumDeclaration*>(s);
            if (e->name == "Color") named = true;
            if (e->name.find("anonymous_enum_") == 0) anon = true;
        }
        assert(enums == 2 && named && anon);
        std::cout << "Test 4 PASSED: enum parsing\n";
        passed++;
    }

    // Test 5: preprocessor directives
    {
        std::string src =
            "#include <stdio.h>\n#include \"x.h\"\n#define MAX 10\n"
            "#define MIN(a,b) ((a)<(b)?(a):(b))\n#pragma once\n";
        auto mod = CParser::parseC(src);
        int includes = 0, macros = 0, pragmas = 0;
        for (auto* s : mod->getChildren("statements")) {
            if (s->conceptType == "IncludeDirective") includes++;
            if (s->conceptType == "MacroDefinition") macros++;
            if (s->conceptType == "PragmaDirective") pragmas++;
        }
        assert(includes == 2 && macros == 2 && pragmas == 1);
        std::cout << "Test 5 PASSED: preprocessor directives\n";
        passed++;
    }

    // Test 6: static/extern qualifiers
    {
        auto mod = CParser::parseC("static int h(){return 1;}\nextern int api(){return 2;}\n");
        auto fns = mod->getChildren("functions");
        assert(fns.size() == 2);
        bool hasStatic = false, hasExtern = false;
        for (auto* f : fns) {
            auto* fn = static_cast<Function*>(f);
            auto* t = fn->getChild("returnType");
            if (!t || t->conceptType != "PrimitiveType") continue;
            auto kind = static_cast<PrimitiveType*>(t)->kind;
            if (fn->name == "h" && kind.find("static") != std::string::npos) hasStatic = true;
            if (fn->name == "api" && kind.find("extern") != std::string::npos) hasExtern = true;
        }
        assert(hasStatic && hasExtern);
        std::cout << "Test 6 PASSED: qualifiers parsed\n";
        passed++;
    }

    // Test 7: function pointers
    {
        auto mod = CParser::parseC("void set_callback(void (*callback)(int)) { }");
        auto fns = mod->getChildren("functions");
        assert(fns.size() == 1);
        auto* fn = static_cast<Function*>(fns[0]);
        auto params = fn->getChildren("parameters");
        assert(params.size() == 1);
        auto* p = static_cast<Parameter*>(params[0]);
        assert(p->name == "callback");
        auto* t = p->getChild("type");
        assert(t != nullptr);
        std::cout << "Test 7 PASSED: function pointers\n";
        passed++;
    }

    // Test 8: pointer variable
    {
        auto mod = CParser::parseC("char* name;");
        auto vars = mod->getChildren("variables");
        assert(vars.size() == 1);
        auto* v = static_cast<Variable*>(vars[0]);
        assert(v->name == "name");
        assert(v->getChild("type") != nullptr);
        std::cout << "Test 8 PASSED: pointer variable\n";
        passed++;
    }

    // Test 9: multiple functions
    {
        auto mod = CParser::parseC("int a(){return 1;}\nint b(){return 2;}\nint c(){return 3;}\n");
        assert(mod->getChildren("functions").size() == 3);
        std::cout << "Test 9 PASSED: multiple functions\n";
        passed++;
    }

    // Test 10: backward compat c++ parser
    {
        auto mod = TreeSitterParser::parseCpp("int cpp_fn() { return 7; }");
        assert(mod != nullptr);
        assert(mod->getChildren("functions").size() == 1);
        std::cout << "Test 10 PASSED: C++ parser unaffected\n";
        passed++;
    }

    // Test 11: C-style comments
    {
        std::string src = "/* x */\n// y\nint live(int z) { return z; } // tail\n";
        auto mod = CParser::parseC(src);
        auto fns = mod->getChildren("functions");
        assert(fns.size() == 1);
        assert(static_cast<Function*>(fns[0])->name == "live");
        std::cout << "Test 11 PASSED: C comments handled\n";
        passed++;
    }

    // Test 12: header guard + pipeline route
    {
        std::string src = "#ifndef X_H\n#define X_H\nint f(void){return 0;}\n#endif\n";
        auto mod = CParser::parseC(src);
        int pragmaLike = 0;
        for (auto* s : mod->getChildren("statements")) if (s->conceptType == "PragmaDirective") pragmaLike++;
        assert(pragmaLike >= 2);

        Pipeline p;
        std::vector<ParseDiagnostic> diags;
        auto parsed = p.parse("int z(void){return 0;}", "c", diags);
        assert(parsed != nullptr && parsed->targetLanguage == "c");
        assert(diags.empty());
        std::cout << "Test 12 PASSED: header guard + pipeline route\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
