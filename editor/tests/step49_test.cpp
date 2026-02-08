// Step 49 TDD Test: Error Recovery and Diagnostics
//
// Tests the ParseResult and ParseDiagnostic types:
// 1. Empty source → empty module, no errors
// 2. Valid Python → module with functions, no diagnostics
// 3. Python syntax error → module + diagnostics with line/column
// 4. Partial parse (valid + broken function) → first function + diagnostics
// 5. C++ syntax error → diagnostics present
// 6. Elisp unbalanced parens → diagnostics present
// 7. ParseResult::hasErrors() returns correct boolean

#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "ast/Parser.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Empty source → empty module, no errors ---
    {
        std::string source = "";
        auto result = TreeSitterParser::parsePythonWithDiagnostics(source);

        assert(result.module != nullptr && "Module should not be null");
        assert(result.module->targetLanguage == "python");
        assert(result.module->getChildren("functions").empty() &&
               "Empty source should have no functions");
        assert(result.diagnostics.empty() &&
               "Empty source should have no diagnostics");
        assert(!result.hasErrors() && "Empty source should have no errors");

        std::cout << "Test 1 PASS: Empty source → empty module, no errors" << std::endl;
        ++passed;
    }

    // --- Test 2: Valid Python → module with functions, no diagnostics ---
    {
        std::string source = "def f(x):\n    return x + 1\n";
        auto result = TreeSitterParser::parsePythonWithDiagnostics(source);

        assert(result.module != nullptr);
        auto functions = result.module->getChildren("functions");
        assert(!functions.empty() && "Should have parsed function");
        assert(result.diagnostics.empty() &&
               "Valid source should have no diagnostics");
        assert(!result.hasErrors());

        std::cout << "Test 2 PASS: Valid Python → module with functions, no diagnostics" << std::endl;
        ++passed;
    }

    // --- Test 3: Python syntax error → diagnostics with line/column ---
    {
        std::string source = "def f(\n";
        auto result = TreeSitterParser::parsePythonWithDiagnostics(source);

        assert(result.module != nullptr && "Module should still be created");
        assert(!result.diagnostics.empty() &&
               "Syntax error should produce diagnostics");
        assert(result.diagnostics[0].severity == "error");
        assert(result.diagnostics[0].line >= 1 && "Line should be >= 1");
        assert(result.diagnostics[0].column >= 1 && "Column should be >= 1");

        std::cout << "Test 3 PASS: Python syntax error → diagnostics" << std::endl;
        ++passed;
    }

    // --- Test 4: Partial parse (valid + broken) → first function + diagnostics ---
    {
        std::string source =
            "def good(x):\n"
            "    return x\n"
            "\n"
            "def bad(\n";
        auto result = TreeSitterParser::parsePythonWithDiagnostics(source);

        assert(result.module != nullptr);
        auto functions = result.module->getChildren("functions");
        assert(!functions.empty() &&
               "Should have parsed at least the first valid function");
        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "good" && "First function should be 'good'");

        assert(!result.diagnostics.empty() &&
               "Broken second function should produce diagnostics");

        std::cout << "Test 4 PASS: Partial parse → first function + diagnostics" << std::endl;
        ++passed;
    }

    // --- Test 5: C++ syntax error → diagnostics present ---
    {
        std::string source = "int f(int x { return x; }";
        auto result = TreeSitterParser::parseCppWithDiagnostics(source);

        assert(result.module != nullptr);
        assert(!result.diagnostics.empty() &&
               "C++ syntax error should produce diagnostics");

        std::cout << "Test 5 PASS: C++ syntax error → diagnostics present" << std::endl;
        ++passed;
    }

    // --- Test 6: Elisp unbalanced parens → diagnostics present ---
    {
        std::string source = "(defun f (x) (+ x 1)";
        auto result = TreeSitterParser::parseElispWithDiagnostics(source);

        assert(result.module != nullptr);
        assert(!result.diagnostics.empty() &&
               "Unbalanced parens should produce diagnostics");

        std::cout << "Test 6 PASS: Elisp unbalanced parens → diagnostics present" << std::endl;
        ++passed;
    }

    // --- Test 7: ParseResult::hasErrors() returns correct boolean ---
    {
        // Valid source
        auto goodResult = TreeSitterParser::parsePythonWithDiagnostics(
            "def f(x):\n    return x\n");
        assert(!goodResult.hasErrors() && "Valid code should not have errors");

        // Invalid source
        auto badResult = TreeSitterParser::parsePythonWithDiagnostics(
            "def f(\n");
        assert(badResult.hasErrors() && "Invalid code should have errors");

        std::cout << "Test 7 PASS: hasErrors() returns correct boolean" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 49 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
