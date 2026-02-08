// Step 48 TDD Test: CST-to-AST Mapping Refinement
//
// Tests more complex constructs across all three languages:
// 1. Python: function with default parameter (def f(x=10))
// 2. Python: if-statement in body
// 3. Python: for-loop in body
// 4. C++: function with multiple statements
// 5. C++: void function
// 6. Elisp: multiple functions in one source
// 7. Python: function with multiple parameters
// 8. C++: function with string literal

#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "ast/Parser.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Python function with default parameter ---
    {
        std::string source = "def f(x=10):\n    return x\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty() && "Should parse function");
        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "f");

        auto params = fn->getChildren("parameters");
        assert(!params.empty() && "Should have parameter");
        auto* param = static_cast<Parameter*>(params[0]);
        assert(param->name == "x" && "Parameter name should be 'x'");

        auto* defVal = param->getChild("defaultValue");
        assert(defVal != nullptr && "Parameter should have a default value");
        assert(defVal->conceptType == "IntegerLiteral" &&
               "Default value should be IntegerLiteral");

        std::cout << "Test 1 PASS: Python default parameter parsed" << std::endl;
        ++passed;
    }

    // --- Test 2: Python if-statement in body ---
    {
        std::string source =
            "def check(x):\n"
            "    if x > 0:\n"
            "        return x\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty());
        auto* fn = static_cast<Function*>(functions[0]);

        auto body = fn->getChildren("body");
        assert(!body.empty() && "Body should not be empty");

        bool hasIf = false;
        for (auto* stmt : body) {
            if (stmt->conceptType == "IfStatement") {
                hasIf = true;
                break;
            }
        }
        assert(hasIf && "Body should contain an IfStatement");

        std::cout << "Test 2 PASS: Python if-statement parsed" << std::endl;
        ++passed;
    }

    // --- Test 3: Python for-loop in body ---
    {
        std::string source =
            "def loop(n):\n"
            "    for i in range(n):\n"
            "        x = i\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty());
        auto* fn = static_cast<Function*>(functions[0]);

        auto body = fn->getChildren("body");
        assert(!body.empty() && "Body should not be empty");

        bool hasFor = false;
        for (auto* stmt : body) {
            if (stmt->conceptType == "ForLoop") {
                hasFor = true;
                break;
            }
        }
        assert(hasFor && "Body should contain a ForLoop");

        std::cout << "Test 3 PASS: Python for-loop parsed" << std::endl;
        ++passed;
    }

    // --- Test 4: C++ function with multiple statements ---
    {
        std::string source =
            "int compute(int a, int b) {\n"
            "    int c = a + b;\n"
            "    return c;\n"
            "}\n";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty());
        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "compute");

        auto body = fn->getChildren("body");
        assert(body.size() >= 2 && "Function should have at least 2 body statements");

        std::cout << "Test 4 PASS: C++ multiple statements parsed" << std::endl;
        ++passed;
    }

    // --- Test 5: C++ void function ---
    {
        std::string source = "void doNothing() {}\n";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty());
        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "doNothing");

        auto* retType = fn->getChild("returnType");
        assert(retType != nullptr && "Function should have a return type");
        auto* primType = static_cast<PrimitiveType*>(retType);
        assert(primType->kind == "void" && "Return type should be 'void'");

        std::cout << "Test 5 PASS: C++ void function parsed" << std::endl;
        ++passed;
    }

    // --- Test 6: Elisp multiple functions in one source ---
    {
        std::string source =
            "(defun add (a b) (+ a b))\n"
            "(defun sub (a b) (- a b))\n";
        auto module = TreeSitterParser::parseElisp(source);

        auto functions = module->getChildren("functions");
        assert(functions.size() >= 2 && "Should parse at least 2 functions");

        auto* fn1 = static_cast<Function*>(functions[0]);
        auto* fn2 = static_cast<Function*>(functions[1]);
        assert(fn1->name == "add" && "First function should be 'add'");
        assert(fn2->name == "sub" && "Second function should be 'sub'");

        std::cout << "Test 6 PASS: Elisp multiple functions parsed" << std::endl;
        ++passed;
    }

    // --- Test 7: Python function with multiple parameters ---
    {
        std::string source = "def add(a, b):\n    return a + b\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty());
        auto* fn = static_cast<Function*>(functions[0]);

        auto params = fn->getChildren("parameters");
        assert(params.size() == 2 && "Function should have exactly 2 parameters");

        auto* p1 = static_cast<Parameter*>(params[0]);
        auto* p2 = static_cast<Parameter*>(params[1]);
        assert(p1->name == "a" && "First parameter should be 'a'");
        assert(p2->name == "b" && "Second parameter should be 'b'");

        std::cout << "Test 7 PASS: Python multiple parameters parsed" << std::endl;
        ++passed;
    }

    // --- Test 8: C++ function with string literal ---
    {
        std::string source =
            "void greet() {\n"
            "    return \"hello\";\n"
            "}\n";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty());
        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "greet");

        auto body = fn->getChildren("body");
        assert(!body.empty() && "Body should not be empty");

        // Find a Return statement with a StringLiteral value
        bool hasStringLit = false;
        for (auto* stmt : body) {
            if (stmt->conceptType == "Return") {
                auto* ret = static_cast<Return*>(stmt);
                auto* val = ret->getChild("value");
                if (val && val->conceptType == "StringLiteral") {
                    hasStringLit = true;
                }
            }
        }
        assert(hasStringLit && "Should contain a Return with StringLiteral");

        std::cout << "Test 8 PASS: C++ string literal parsed" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 48 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
