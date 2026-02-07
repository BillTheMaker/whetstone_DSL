// Step 39 TDD Test: Basic C++ generator skeleton
//
// Tests that CppGenerator (not yet implemented) can:
// 1. Inherit from ProjectionGenerator
// 2. Generate a namespace wrapper from a Module
// 3. Generate a C++ function signature from a Function with parameters
// 4. NOT produce Python syntax (def, colons)
//
// This test will fail to compile until CppGenerator is implemented in Generator.h.

#include <iostream>
#include <string>
#include <cassert>
#include "ast/Generator.h"

// Helper to check if a string contains a substring
static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// Helper to check a string does NOT contain a substring
static bool notContains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) == std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: CppGenerator is a ProjectionGenerator ---
    {
        CppGenerator gen;
        ProjectionGenerator* base = &gen;
        (void)base;  // Compiles only if CppGenerator inherits from ProjectionGenerator
        std::cout << "Test 1 PASS: CppGenerator inherits from ProjectionGenerator" << std::endl;
        ++passed;
    }

    // --- Test 2: Empty module produces namespace ---
    {
        Module mod("m1", "Calculator", "cpp");
        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert(contains(output, "namespace Calculator") &&
               "Module should produce a namespace declaration");
        assert(contains(output, "{") && "Namespace should have opening brace");
        assert(contains(output, "}") && "Namespace should have closing brace");

        std::cout << "Test 2 PASS: Empty module produces namespace with braces" << std::endl;
        ++passed;
    }

    // --- Test 3: Module with a function produces C++ function signature ---
    {
        Module mod("m1", "Calculator", "cpp");

        Function* addFn = new Function("f1", "add");

        // Two int parameters
        Parameter* paramX = new Parameter("p1", "x");
        PrimitiveType* intTypeX = new PrimitiveType("t1", "int");
        paramX->setChild("type", intTypeX);

        Parameter* paramY = new Parameter("p2", "y");
        PrimitiveType* intTypeY = new PrimitiveType("t2", "int");
        paramY->setChild("type", intTypeY);

        addFn->addChild("parameters", paramX);
        addFn->addChild("parameters", paramY);

        // Add a return type to the function
        PrimitiveType* returnType = new PrimitiveType("t3", "int");
        addFn->setChild("returnType", returnType);

        mod.addChild("functions", addFn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        // Should contain C++ function signature elements
        assert(contains(output, "int") && "Output should contain 'int' type");
        assert(contains(output, "add") && "Output should contain function name 'add'");
        assert(contains(output, "(") && "Output should contain opening parenthesis");
        assert(contains(output, ")") && "Output should contain closing parenthesis");
        assert(contains(output, "x") && "Output should contain parameter 'x'");
        assert(contains(output, "y") && "Output should contain parameter 'y'");
        assert(contains(output, "{") && "Function body should have opening brace");
        assert(contains(output, "}") && "Function body should have closing brace");

        std::cout << "Test 3 PASS: Function produces C++ signature with types and braces" << std::endl;
        ++passed;

        delete returnType;
        delete intTypeY;
        delete paramY;
        delete intTypeX;
        delete paramX;
        delete addFn;
    }

    // --- Test 4: C++ output does NOT contain Python syntax ---
    {
        Module mod("m1", "Calculator", "cpp");

        Function* fn = new Function("f1", "compute");
        Parameter* param = new Parameter("p1", "n");
        PrimitiveType* paramType = new PrimitiveType("t1", "int");
        param->setChild("type", paramType);
        fn->addChild("parameters", param);

        PrimitiveType* retType = new PrimitiveType("t2", "int");
        fn->setChild("returnType", retType);

        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert(notContains(output, "def ") && "C++ output must not contain 'def '");
        assert(notContains(output, "pass") && "C++ output must not contain 'pass'");
        // Python uses colon after function signature; C++ uses brace
        // (Note: colons can appear in C++ namespaces, so we check specifically for "def ...:")
        assert(notContains(output, "def ") && "C++ output must not contain Python-style function definitions");

        std::cout << "Test 4 PASS: C++ output contains no Python syntax" << std::endl;
        ++passed;

        delete retType;
        delete paramType;
        delete param;
        delete fn;
    }

    // --- Test 5: generate(nullptr) returns empty string ---
    {
        CppGenerator gen;
        std::string output = gen.generate(nullptr);
        assert(output.empty() && "generate(nullptr) should return empty string");

        std::cout << "Test 5 PASS: generate(nullptr) returns empty string" << std::endl;
        ++passed;
    }

    // --- Test 6: Module with multiple functions ---
    {
        Module mod("m1", "MathLib", "cpp");

        Function* fn1 = new Function("f1", "add");
        PrimitiveType* ret1 = new PrimitiveType("t1", "int");
        fn1->setChild("returnType", ret1);

        Function* fn2 = new Function("f2", "subtract");
        PrimitiveType* ret2 = new PrimitiveType("t2", "int");
        fn2->setChild("returnType", ret2);

        mod.addChild("functions", fn1);
        mod.addChild("functions", fn2);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert(contains(output, "add") && "Output should contain first function 'add'");
        assert(contains(output, "subtract") && "Output should contain second function 'subtract'");
        assert(contains(output, "namespace MathLib") && "Output should contain namespace 'MathLib'");

        std::cout << "Test 6 PASS: Module with multiple functions generates all of them" << std::endl;
        ++passed;

        delete ret2;
        delete fn2;
        delete ret1;
        delete fn1;
    }

    // --- Summary ---
    std::cout << "\n=== Step 39 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
