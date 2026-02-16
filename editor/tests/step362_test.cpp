// Step 362: C Generator (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "ast/CGenerator.h"
#include "Pipeline.h"
#include "ast/ClassDeclaration.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/PreprocessorNodes.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;

    // Test 1: function generation
    {
        CGenerator gen;
        Function fn("f1", "add");
        fn.setChild("returnType", new PrimitiveType("t1", "int"));
        fn.addChild("parameters", new Parameter("p1", "x"));
        auto* ret = new Return();
        ret->id = "r1";
        ret->setChild("value", new IntegerLiteral("i1", 1));
        fn.addChild("body", ret);
        auto out = gen.generate(&fn);
        assert(out.find("int add(") != std::string::npos);
        assert(out.find("return 1;") != std::string::npos);
        std::cout << "Test 1 PASSED: function generation\n";
        passed++;
    }

    // Test 2: struct output from class
    {
        CGenerator gen;
        ClassDeclaration cls("c1", "Point");
        auto* x = new Variable("v1", "x");
        x->setChild("type", new PrimitiveType("t1", "int"));
        cls.addChild("fields", x);
        auto out = gen.generate(&cls);
        assert(out.find("typedef struct Point") != std::string::npos);
        assert(out.find("int x;") != std::string::npos);
        std::cout << "Test 2 PASSED: struct output\n";
        passed++;
    }

    // Test 3: typedef output
    {
        CGenerator gen;
        TypeAlias ta("ta1", "Count", "unsigned int", false);
        auto out = gen.generate(&ta);
        assert(out == "typedef unsigned int Count;");
        std::cout << "Test 3 PASSED: typedef output\n";
        passed++;
    }

    // Test 4: enum output
    {
        CGenerator gen;
        EnumDeclaration e("e1", "Color");
        e.addChild("members", new EnumMember("m1", "RED", "1"));
        e.addChild("members", new EnumMember("m2", "GREEN", "2"));
        auto out = gen.generate(&e);
        assert(out.find("enum Color") != std::string::npos);
        assert(out.find("RED = 1") != std::string::npos);
        std::cout << "Test 4 PASSED: enum output\n";
        passed++;
    }

    // Test 5: preprocessor output
    {
        CGenerator gen;
        IncludeDirective inc("i1", "stdio.h", true);
        MacroDefinition mac("m1", "MAX");
        mac.body = "100";
        auto a = gen.generate(&inc);
        auto b = gen.generate(&mac);
        assert(a == "#include <stdio.h>");
        assert(b.find("#define MAX 100") != std::string::npos);
        std::cout << "Test 5 PASSED: preprocessor output\n";
        passed++;
    }

    // Test 6: pointer type preserved
    {
        CGenerator gen;
        Variable v("v1", "name");
        v.setChild("type", new PrimitiveType("t1", "char*"));
        auto out = gen.generate(&v);
        assert(out.find("char* name;") != std::string::npos);
        std::cout << "Test 6 PASSED: pointer type\n";
        passed++;
    }

    // Test 7: owner/manual annotation comment
    {
        CGenerator gen;
        Function fn("f1", "alloc");
        fn.setChild("returnType", new PrimitiveType("t1", "void*"));
        auto* owner = new OwnerAnnotation("a1", "Manual");
        fn.addChild("annotations", owner);
        auto out = gen.generate(&fn);
        assert(out.find("@owner") != std::string::npos || out.find("Owner") != std::string::npos);
        std::cout << "Test 7 PASSED: owner annotation comment\n";
        passed++;
    }

    // Test 8: class (python-like) to c struct generation
    {
        CGenerator gen;
        ClassDeclaration cls("c1", "Widget");
        auto out = gen.generate(&cls);
        assert(out.find("typedef struct Widget") != std::string::npos);
        std::cout << "Test 8 PASSED: class to struct generation\n";
        passed++;
    }

    // Test 9: c++ method to c-style function
    {
        CGenerator gen;
        MethodDeclaration m("m1", "render");
        m.className = "Widget";
        auto out = gen.generate(&m);
        assert(out.find("Widget_render(") != std::string::npos);
        std::cout << "Test 9 PASSED: method to c function naming\n";
        passed++;
    }

    // Test 10: commentPrefix
    {
        CGenerator gen;
        assert(gen.commentPrefix() == "// ");
        std::cout << "Test 10 PASSED: comment prefix\n";
        passed++;
    }

    // Test 11: C string literal + null
    {
        CGenerator gen;
        StringLiteral s("s1", "abc");
        NullLiteral n;
        auto a = gen.generate(&s);
        auto b = gen.generate(&n);
        assert(a == "\"abc\"");
        assert(b == "NULL");
        std::cout << "Test 11 PASSED: C string/null literal\n";
        passed++;
    }

    // Test 12: pipeline generate route for c
    {
        Pipeline p;
        Function fn("f1", "work");
        std::string out = p.generate(&fn, "c");
        assert(!out.empty());
        assert(out.find("work(") != std::string::npos);
        std::cout << "Test 12 PASSED: pipeline generate route c\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
