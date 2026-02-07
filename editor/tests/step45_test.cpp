// Step 45 TDD Test: Tree-sitter Python integration
//
// Tests that TreeSitterParser::parsePython() actually parses real Python source
// into a correct SemAnno AST, not just returning placeholder stubs:
// 1. Simple function → Function node with correct name
// 2. Parameters → Parameter nodes with correct names
// 3. Return statement → Return node with value
// 4. Binary operation → BinaryOperation with correct operator
// 5. Auto-annotates with @Reclaim(Tracing) since Python uses tracing GC
// 6. Module targetLanguage is "python"
//
// Will fail until TreeSitterParser::parsePython() is replaced with real
// tree-sitter-python integration.

#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "ast/Parser.h"
#include "ast/Annotation.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Parse simple Python function ---
    {
        std::string source = "def f(x):\n    return x + 1\n";
        auto module = TreeSitterParser::parsePython(source);

        assert(module != nullptr && "parsePython should return a non-null module");
        assert(module->targetLanguage == "python" && "Module target language should be 'python'");

        std::cout << "Test 1 PASS: parsePython returns non-null module with python target" << std::endl;
        ++passed;
    }

    // --- Test 2: Parsed function has correct name ---
    {
        std::string source = "def f(x):\n    return x + 1\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty() && "Module should contain at least one function");

        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "f" && "Function name should be 'f'");

        std::cout << "Test 2 PASS: Parsed function has name 'f'" << std::endl;
        ++passed;
    }

    // --- Test 3: Function has parameter ---
    {
        std::string source = "def f(x):\n    return x + 1\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

        auto params = fn->getChildren("parameters");
        assert(!params.empty() && "Function should have at least one parameter");

        auto* param = static_cast<Parameter*>(params[0]);
        assert(param->name == "x" && "Parameter name should be 'x'");

        std::cout << "Test 3 PASS: Function has parameter 'x'" << std::endl;
        ++passed;
    }

    // --- Test 4: Function body contains Return with BinaryOperation ---
    {
        std::string source = "def f(x):\n    return x + 1\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

        auto body = fn->getChildren("body");
        assert(!body.empty() && "Function body should not be empty");

        // Find the Return statement
        bool hasReturn = false;
        for (auto* stmt : body) {
            if (stmt->conceptType == "Return") {
                hasReturn = true;
                auto* ret = static_cast<Return*>(stmt);
                auto* val = ret->getChild("value");
                assert(val != nullptr && "Return should have a value");
                assert(val->conceptType == "BinaryOperation" &&
                       "Return value should be a BinaryOperation");

                auto* binOp = static_cast<BinaryOperation*>(val);
                assert(binOp->op == "+" && "Operator should be '+'");
                break;
            }
        }
        assert(hasReturn && "Body should contain a Return statement");

        std::cout << "Test 4 PASS: Return with BinaryOperation(+) found" << std::endl;
        ++passed;
    }

    // --- Test 5: Python module auto-annotated with @Reclaim(Tracing) ---
    {
        std::string source = "def f(x):\n    return x + 1\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

        // Check for memory annotation on the function
        auto annotations = fn->getChildren("annotations");
        bool hasReclaimTracing = false;
        for (auto* anno : annotations) {
            if (anno->conceptType == "ReclaimAnnotation") {
                auto* reclaim = static_cast<ReclaimAnnotation*>(anno);
                if (reclaim->strategy == "Tracing") {
                    hasReclaimTracing = true;
                }
            }
        }
        assert(hasReclaimTracing &&
               "Python functions should be auto-annotated with @Reclaim(Tracing)");

        std::cout << "Test 5 PASS: Python function auto-annotated with @Reclaim(Tracing)" << std::endl;
        ++passed;
    }

    // --- Test 6: Multiple functions parsed ---
    {
        std::string source =
            "def add(a, b):\n"
            "    return a + b\n"
            "\n"
            "def sub(a, b):\n"
            "    return a - b\n";
        auto module = TreeSitterParser::parsePython(source);

        auto functions = module->getChildren("functions");
        assert(functions.size() >= 2 && "Should parse at least 2 functions");

        auto* fn1 = static_cast<Function*>(functions[0]);
        auto* fn2 = static_cast<Function*>(functions[1]);
        assert(fn1->name == "add" && "First function should be 'add'");
        assert(fn2->name == "sub" && "Second function should be 'sub'");

        std::cout << "Test 6 PASS: Multiple functions parsed correctly" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 45 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
