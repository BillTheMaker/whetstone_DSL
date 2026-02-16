// Step 340: C++ Generator — Preprocessor, Enum, Namespace (12 tests)
// Tests code generation for preprocessor directives, enums, namespaces, type aliases
// across all 10 generators (C++, Python, Java, Rust, Go, JS, Elisp, Kotlin, C#, + TS inherits JS)

#include <cassert>
#include <iostream>
#include <string>
#include "ast/PreprocessorNodes.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/Serialization.h"
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/JavaGenerator.h"
#include "ast/RustGenerator.h"
#include "ast/GoGenerator.h"
#include "ast/JavaScriptGenerator.h"
#include "ast/ElispGenerator.h"
#include "ast/KotlinGenerator.h"
#include "ast/CSharpGenerator.h"

int main() {
    int passed = 0;

    // Test 1: C++ #include output (system vs local)
    {
        CppGenerator gen;
        IncludeDirective inc1; inc1.path = "vector"; inc1.isSystem = true;
        IncludeDirective inc2; inc2.path = "ast/ASTNode.h"; inc2.isSystem = false;
        std::string out1 = gen.generate(&inc1);
        std::string out2 = gen.generate(&inc2);
        assert(out1 == "#include <vector>");
        assert(out2 == "#include \"ast/ASTNode.h\"");
        std::cout << "Test 1 PASSED: C++ #include output\n";
        passed++;
    }

    // Test 2: C++ enum class output
    {
        CppGenerator gen;
        EnumDeclaration e; e.name = "Color"; e.isScoped = true; e.underlyingType = "int";
        e.addChild("members", new EnumMember("m1", "Red", "0"));
        e.addChild("members", new EnumMember("m2", "Green", "1"));
        e.addChild("members", new EnumMember("m3", "Blue", "2"));
        std::string out = gen.generate(&e);
        assert(out.find("enum class Color : int") != std::string::npos);
        assert(out.find("Red = 0") != std::string::npos);
        assert(out.find("Green = 1") != std::string::npos);
        assert(out.find("Blue = 2") != std::string::npos);
        std::cout << "Test 2 PASSED: C++ enum class output\n";
        passed++;
    }

    // Test 3: C++ namespace output
    {
        CppGenerator gen;
        NamespaceDeclaration ns; ns.name = "whetstone";
        auto* func = new Function(); func->name = "helper";
        ns.addChild("body", func);
        std::string out = gen.generate(&ns);
        assert(out.find("namespace whetstone {") != std::string::npos);
        assert(out.find("helper") != std::string::npos);
        assert(out.find("}") != std::string::npos);
        std::cout << "Test 3 PASSED: C++ namespace output\n";
        passed++;
    }

    // Test 4: Python enum → class(Enum) output
    {
        PythonGenerator gen;
        EnumDeclaration e; e.name = "Color"; e.isScoped = true;
        e.addChild("members", new EnumMember("m1", "RED", "1"));
        e.addChild("members", new EnumMember("m2", "GREEN", "2"));
        std::string out = gen.generate(&e);
        assert(out.find("class Color(Enum)") != std::string::npos);
        assert(out.find("RED = 1") != std::string::npos);
        assert(out.find("GREEN = 2") != std::string::npos);
        std::cout << "Test 4 PASSED: Python enum output\n";
        passed++;
    }

    // Test 5: Java enum output
    {
        JavaGenerator gen;
        EnumDeclaration e; e.name = "Status";
        e.addChild("members", new EnumMember("m1", "ACTIVE"));
        e.addChild("members", new EnumMember("m2", "INACTIVE"));
        std::string out = gen.generate(&e);
        assert(out.find("enum Status") != std::string::npos);
        assert(out.find("ACTIVE") != std::string::npos);
        assert(out.find("INACTIVE") != std::string::npos);
        std::cout << "Test 5 PASSED: Java enum output\n";
        passed++;
    }

    // Test 6: Rust enum output
    {
        RustGenerator gen;
        EnumDeclaration e; e.name = "Direction";
        e.addChild("members", new EnumMember("m1", "North", "0"));
        e.addChild("members", new EnumMember("m2", "South", "1"));
        std::string out = gen.generate(&e);
        assert(out.find("enum Direction") != std::string::npos);
        assert(out.find("North = 0") != std::string::npos);
        assert(out.find("South = 1") != std::string::npos);
        std::cout << "Test 6 PASSED: Rust enum output\n";
        passed++;
    }

    // Test 7: Go enum → iota output
    {
        GoGenerator gen;
        EnumDeclaration e; e.name = "Color";
        e.addChild("members", new EnumMember("m1", "Red"));
        e.addChild("members", new EnumMember("m2", "Green"));
        std::string out = gen.generate(&e);
        assert(out.find("type Color int") != std::string::npos);
        assert(out.find("iota") != std::string::npos);
        assert(out.find("Red") != std::string::npos);
        std::cout << "Test 7 PASSED: Go enum → iota output\n";
        passed++;
    }

    // Test 8: Cross-language include adaptation (C++ → Python, Java, Rust, Go, JS)
    {
        IncludeDirective inc; inc.path = "ast/ASTNode.h"; inc.isSystem = false;

        PythonGenerator pyGen;
        assert(pyGen.generate(&inc).find("import") != std::string::npos);

        JavaGenerator javaGen;
        assert(javaGen.generate(&inc).find("import") != std::string::npos);

        RustGenerator rustGen;
        assert(rustGen.generate(&inc).find("use") != std::string::npos);

        GoGenerator goGen;
        assert(goGen.generate(&inc).find("import") != std::string::npos);

        JavaScriptGenerator jsGen;
        assert(jsGen.generate(&inc).find("import") != std::string::npos);

        ElispGenerator elispGen;
        assert(elispGen.generate(&inc).find("require") != std::string::npos);

        KotlinGenerator ktGen;
        assert(ktGen.generate(&inc).find("import") != std::string::npos);

        CSharpGenerator csGen;
        assert(csGen.generate(&inc).find("using") != std::string::npos);

        std::cout << "Test 8 PASSED: Cross-language include adaptation (8 generators)\n";
        passed++;
    }

    // Test 9: C++ using alias and typedef
    {
        CppGenerator gen;
        TypeAlias ta1; ta1.aliasName = "NodePtr"; ta1.targetType = "std::shared_ptr<ASTNode>"; ta1.isUsing = true;
        TypeAlias ta2; ta2.aliasName = "NodeVec"; ta2.targetType = "std::vector<ASTNode*>"; ta2.isUsing = false;
        assert(gen.generate(&ta1) == "using NodePtr = std::shared_ptr<ASTNode>;");
        assert(gen.generate(&ta2) == "typedef std::vector<ASTNode*> NodeVec;");
        std::cout << "Test 9 PASSED: C++ using alias and typedef\n";
        passed++;
    }

    // Test 10: Kotlin enum class + C# enum output
    {
        KotlinGenerator ktGen;
        EnumDeclaration e1; e1.name = "Level";
        e1.addChild("members", new EnumMember("m1", "LOW"));
        e1.addChild("members", new EnumMember("m2", "HIGH"));
        std::string ktOut = ktGen.generate(&e1);
        assert(ktOut.find("enum class Level") != std::string::npos);
        assert(ktOut.find("LOW") != std::string::npos);

        CSharpGenerator csGen;
        EnumDeclaration e2; e2.name = "Level"; e2.underlyingType = "byte";
        e2.addChild("members", new EnumMember("m1", "Low", "1"));
        e2.addChild("members", new EnumMember("m2", "High", "2"));
        std::string csOut = csGen.generate(&e2);
        assert(csOut.find("enum Level : byte") != std::string::npos);
        assert(csOut.find("Low = 1") != std::string::npos);

        std::cout << "Test 10 PASSED: Kotlin + C# enum output\n";
        passed++;
    }

    // Test 11: C++ #pragma and #define output
    {
        CppGenerator gen;
        PragmaDirective prag; prag.directive = "once";
        assert(gen.generate(&prag) == "#pragma once");

        MacroDefinition mac; mac.name = "MAX"; mac.isFunctionLike = true;
        mac.parameters = {"a", "b"};
        mac.body = "((a) > (b) ? (a) : (b))";
        std::string macOut = gen.generate(&mac);
        assert(macOut.find("#define MAX(a, b)") != std::string::npos);
        assert(macOut.find("((a) > (b) ? (a) : (b))") != std::string::npos);

        std::cout << "Test 11 PASSED: C++ #pragma and #define output\n";
        passed++;
    }

    // Test 12: All 9 generators produce non-empty output for all 6 new types
    {
        IncludeDirective inc; inc.path = "header.h"; inc.isSystem = false;
        PragmaDirective prag; prag.directive = "once";
        MacroDefinition mac; mac.name = "FOO"; mac.body = "42";
        EnumDeclaration enumDecl; enumDecl.name = "E";
        enumDecl.addChild("members", new EnumMember("m1", "A"));
        NamespaceDeclaration ns; ns.name = "ns";
        TypeAlias ta; ta.aliasName = "Alias"; ta.targetType = "int"; ta.isUsing = true;

        const ASTNode* nodes[] = {&inc, &prag, &mac, &enumDecl, &ns, &ta};

        CppGenerator cppGen;
        PythonGenerator pyGen;
        JavaGenerator javaGen;
        RustGenerator rustGen;
        GoGenerator goGen;
        JavaScriptGenerator jsGen;
        ElispGenerator elispGen;
        KotlinGenerator ktGen;
        CSharpGenerator csGen;

        ProjectionGenerator* gens[] = {&cppGen, &pyGen, &javaGen, &rustGen, &goGen, &jsGen, &elispGen, &ktGen, &csGen};

        int nonEmpty = 0;
        for (int g = 0; g < 9; g++) {
            for (int n = 0; n < 6; n++) {
                std::string out = gens[g]->generate(nodes[n]);
                if (!out.empty()) nonEmpty++;
            }
        }
        assert(nonEmpty == 54);
        std::cout << "Test 12 PASSED: All 9 generators produce non-empty output for all 6 types (" << nonEmpty << "/54)\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
