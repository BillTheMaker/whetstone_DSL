// Step 341: Phase 12d Integration + Sprint 12 Summary (8 tests)
// Full C++ depth validation: parse → AST → generate roundtrips, cross-language,
// combined workflow + C++ constructs, sprint 12 totals verification

#include <cassert>
#include <iostream>
#include <string>
#include <set>
#include "ast/PreprocessorNodes.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
#include "ast/Parser.h"
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/JavaGenerator.h"
#include "ast/RustGenerator.h"
#include "ast/GoGenerator.h"
#include "ast/JavaScriptGenerator.h"
#include "ast/ElispGenerator.h"
#include "ast/KotlinGenerator.h"
#include "ast/CSharpGenerator.h"
#include "CompactAST.h"

int main() {
    int passed = 0;

    // Test 1: Parse realistic C++ header: includes + pragma + namespace + enum + class + function
    {
        std::string src = R"(
#pragma once
#include <string>
#include "ast/ASTNode.h"

namespace whetstone {

enum class NodeKind : int {
    Function = 0,
    Variable = 1,
    Class = 2
};

class ASTVisitor {
public:
    virtual void visit() {}
};

void helper(int x) { return; }

}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");

        // Collect all concept types present
        std::set<std::string> types;
        for (auto* s : stmts) types.insert(s->conceptType);

        assert(types.count("PragmaDirective") || types.count("IncludeDirective"));
        assert(types.count("NamespaceDeclaration"));

        // Namespace should contain enum, class, function
        bool foundNs = false;
        for (auto* s : stmts) {
            if (s->conceptType == "NamespaceDeclaration") {
                foundNs = true;
                auto body = s->getChildren("body");
                std::set<std::string> bodyTypes;
                for (auto* b : body) bodyTypes.insert(b->conceptType);
                assert(bodyTypes.count("EnumDeclaration"));
                assert(bodyTypes.count("ClassDeclaration"));
                assert(bodyTypes.count("Function"));
            }
        }
        assert(foundNs);
        std::cout << "Test 1 PASSED: Parse realistic C++ header with all constructs\n";
        passed++;
    }

    // Test 2: Round-trip: C++ header → AST → JSON → fromJson → generate → verify structure
    {
        std::string src = R"(
enum class Color : int {
    Red = 0,
    Green = 1
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool foundEnum = false;
        for (auto* s : stmts) {
            if (s->conceptType == "EnumDeclaration") {
                foundEnum = true;
                // JSON roundtrip
                nlohmann::json j = propertiesToJson(s);
                auto* restored = createNode(s->conceptType);
                setPropertiesFromJson(restored, j);
                // Verify properties survived
                auto* orig = static_cast<EnumDeclaration*>(s);
                auto* rest = static_cast<EnumDeclaration*>(restored);
                assert(rest->name == orig->name);
                assert(rest->isScoped == orig->isScoped);
                delete restored;
            }
        }
        assert(foundEnum);
        std::cout << "Test 2 PASSED: Enum parse → JSON roundtrip → verify\n";
        passed++;
    }

    // Test 3: Enum inside namespace: parse → verify scoping in AST
    {
        std::string src = R"(
namespace outer {
    enum class Status { Active, Inactive };
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "NamespaceDeclaration") {
                auto* ns = static_cast<NamespaceDeclaration*>(s);
                assert(ns->name == "outer");
                auto body = ns->getChildren("body");
                bool hasEnum = false;
                for (auto* b : body) {
                    if (b->conceptType == "EnumDeclaration") {
                        hasEnum = true;
                        auto* e = static_cast<EnumDeclaration*>(b);
                        assert(e->name == "Status");
                        assert(e->isScoped);
                    }
                }
                assert(hasEnum);
                found = true;
            }
        }
        assert(found);
        std::cout << "Test 3 PASSED: Enum inside namespace scoping\n";
        passed++;
    }

    // Test 4: Cross-language: C++ enum → Java enum + Python class(Enum) + Rust enum
    {
        EnumDeclaration e; e.name = "Color"; e.isScoped = true;
        e.addChild("members", new EnumMember("m1", "Red", "0"));
        e.addChild("members", new EnumMember("m2", "Green", "1"));
        e.addChild("members", new EnumMember("m3", "Blue", "2"));

        CppGenerator cppGen;
        std::string cppOut = cppGen.generate(&e);
        assert(cppOut.find("enum class Color") != std::string::npos);

        JavaGenerator javaGen;
        std::string javaOut = javaGen.generate(&e);
        assert(javaOut.find("enum Color") != std::string::npos);
        assert(javaOut.find("Red") != std::string::npos);

        PythonGenerator pyGen;
        std::string pyOut = pyGen.generate(&e);
        assert(pyOut.find("class Color(Enum)") != std::string::npos);

        RustGenerator rustGen;
        std::string rustOut = rustGen.generate(&e);
        assert(rustOut.find("enum Color") != std::string::npos);

        KotlinGenerator ktGen;
        std::string ktOut = ktGen.generate(&e);
        assert(ktOut.find("enum class Color") != std::string::npos);

        CSharpGenerator csGen;
        std::string csOut = csGen.generate(&e);
        assert(csOut.find("enum Color") != std::string::npos);

        std::cout << "Test 4 PASSED: C++ enum → 6 language outputs\n";
        passed++;
    }

    // Test 5: Combined namespace + class + enum + function → full pipeline
    {
        NamespaceDeclaration ns; ns.name = "ast";

        auto* cls = new ClassDeclaration(); cls->name = "Node"; cls->isAbstract = true;
        auto* meth = new Function(); meth->name = "accept";
        cls->addChild("methods", meth);
        ns.addChild("body", cls);

        auto* e = new EnumDeclaration(); e->name = "Kind"; e->isScoped = true;
        e->addChild("members", new EnumMember("m1", "Expr"));
        e->addChild("members", new EnumMember("m2", "Stmt"));
        ns.addChild("body", e);

        auto* func = new Function(); func->name = "createNode";
        ns.addChild("body", func);

        CppGenerator gen;
        std::string out = gen.generate(&ns);
        assert(out.find("namespace ast") != std::string::npos);
        assert(out.find("Node") != std::string::npos);
        assert(out.find("accept") != std::string::npos);
        assert(out.find("enum class Kind") != std::string::npos);
        assert(out.find("Expr") != std::string::npos);
        assert(out.find("createNode") != std::string::npos);

        std::cout << "Test 5 PASSED: Combined namespace + class + enum + function pipeline\n";
        passed++;
    }

    // Test 6: All new AST nodes serialize/deserialize correctly (batch verification)
    {
        // IncludeDirective
        IncludeDirective inc; inc.path = "vector"; inc.isSystem = true;
        nlohmann::json j1 = propertiesToJson(&inc);
        auto* r1 = createNode("IncludeDirective");
        setPropertiesFromJson(r1, j1);
        assert(static_cast<IncludeDirective*>(r1)->path == "vector");
        assert(static_cast<IncludeDirective*>(r1)->isSystem);

        // PragmaDirective
        PragmaDirective prag; prag.directive = "once";
        nlohmann::json j2 = propertiesToJson(&prag);
        auto* r2 = createNode("PragmaDirective");
        setPropertiesFromJson(r2, j2);
        assert(static_cast<PragmaDirective*>(r2)->directive == "once");

        // MacroDefinition
        MacroDefinition mac; mac.name = "MAX"; mac.isFunctionLike = true;
        mac.parameters = {"a", "b"}; mac.body = "((a)>(b)?(a):(b))";
        nlohmann::json j3 = propertiesToJson(&mac);
        auto* r3 = createNode("MacroDefinition");
        setPropertiesFromJson(r3, j3);
        auto* rm = static_cast<MacroDefinition*>(r3);
        assert(rm->name == "MAX");
        assert(rm->isFunctionLike);
        assert(rm->parameters.size() == 2);

        // EnumDeclaration
        EnumDeclaration ed; ed.name = "Color"; ed.isScoped = true; ed.underlyingType = "int";
        nlohmann::json j4 = propertiesToJson(&ed);
        auto* r4 = createNode("EnumDeclaration");
        setPropertiesFromJson(r4, j4);
        auto* re = static_cast<EnumDeclaration*>(r4);
        assert(re->name == "Color");
        assert(re->isScoped);
        assert(re->underlyingType == "int");

        // NamespaceDeclaration
        NamespaceDeclaration nsd; nsd.name = "whetstone";
        nlohmann::json j5 = propertiesToJson(&nsd);
        auto* r5 = createNode("NamespaceDeclaration");
        setPropertiesFromJson(r5, j5);
        assert(static_cast<NamespaceDeclaration*>(r5)->name == "whetstone");

        // TypeAlias
        TypeAlias ta; ta.aliasName = "Ptr"; ta.targetType = "int*"; ta.isUsing = true;
        nlohmann::json j6 = propertiesToJson(&ta);
        auto* r6 = createNode("TypeAlias");
        setPropertiesFromJson(r6, j6);
        auto* rt = static_cast<TypeAlias*>(r6);
        assert(rt->aliasName == "Ptr");
        assert(rt->targetType == "int*");
        assert(rt->isUsing);

        delete r1; delete r2; delete r3; delete r4; delete r5; delete r6;
        std::cout << "Test 6 PASSED: All 6 new AST nodes serialize/deserialize correctly\n";
        passed++;
    }

    // Test 7: CompactAST names for all new node types
    {
        IncludeDirective inc; inc.path = "vector"; inc.isSystem = true;
        PragmaDirective prag; prag.directive = "once";
        MacroDefinition mac; mac.name = "MAX";
        EnumDeclaration ed; ed.name = "Color";
        EnumMember em("m1", "Red", "0");
        NamespaceDeclaration nsd; nsd.name = "whetstone";
        TypeAlias ta; ta.aliasName = "Ptr";

        assert(getNodeName(&inc) == "vector");
        assert(getNodeName(&prag) == "once");
        assert(getNodeName(&mac) == "MAX");
        assert(getNodeName(&ed) == "Color");
        assert(getNodeName(&em) == "Red");
        assert(getNodeName(&nsd) == "whetstone");
        assert(getNodeName(&ta) == "Ptr");

        std::cout << "Test 7 PASSED: CompactAST names for all new node types\n";
        passed++;
    }

    // Test 8: C++ self-hosting progress — parse simplified Whetstone header fragment
    {
        std::string src = R"(
#pragma once
#include <string>
#include <vector>
#include "ASTNode.h"

namespace whetstone {

enum class NodeKind {
    Module,
    Function,
    Variable
};

class ProjectionGenerator {
public:
    virtual std::string generate() {}
};

class CppGenerator : public ProjectionGenerator {
public:
    std::string generate() {}
};

}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto stmts = mod->getChildren("statements");

        // Count constructs found
        int includes = 0, pragmas = 0, namespaces = 0;
        for (auto* s : stmts) {
            if (s->conceptType == "IncludeDirective") includes++;
            if (s->conceptType == "PragmaDirective") pragmas++;
            if (s->conceptType == "NamespaceDeclaration") namespaces++;
        }
        assert(includes >= 2);  // <string>, <vector>, "ASTNode.h"
        assert(pragmas >= 1);
        assert(namespaces >= 1);

        // Inside namespace: enum + 2 classes
        for (auto* s : stmts) {
            if (s->conceptType == "NamespaceDeclaration") {
                auto body = s->getChildren("body");
                int enums = 0, classes = 0;
                for (auto* b : body) {
                    if (b->conceptType == "EnumDeclaration") enums++;
                    if (b->conceptType == "ClassDeclaration") classes++;
                }
                assert(enums >= 1);
                assert(classes >= 2);

                // CppGenerator should inherit from ProjectionGenerator
                for (auto* b : body) {
                    if (b->conceptType == "ClassDeclaration") {
                        auto* cls = static_cast<ClassDeclaration*>(b);
                        if (cls->name == "CppGenerator") {
                            auto bases = cls->getBases();
                            assert(!bases.empty());
                            assert(bases[0].name == "ProjectionGenerator");
                        }
                    }
                }
            }
        }

        std::cout << "Test 8 PASSED: Self-hosting progress — Whetstone header fragment parsed\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
