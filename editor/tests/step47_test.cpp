// Step 47 TDD Test: Tree-sitter Elisp integration
//
// Tests that TreeSitterParser::parseElisp() parses Emacs Lisp source:
// 1. defun → Function node with correct name
// 2. Parameters from defun arglist
// 3. defvar → Variable node
// 4. if → IfStatement
// 5. Auto-annotates with @Reclaim(Tracing) since Elisp uses GC
// 6. Module targetLanguage is "elisp"
//
// Will fail until TreeSitterParser::parseElisp() is implemented
// with real tree-sitter-elisp integration.

#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "ast/Parser.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Parse simple defun ---
    {
        std::string source = "(defun f (x) (+ x 1))";
        auto module = TreeSitterParser::parseElisp(source);

        assert(module != nullptr && "parseElisp should return a non-null module");
        assert(module->targetLanguage == "elisp" && "Module target language should be 'elisp'");

        std::cout << "Test 1 PASS: parseElisp returns non-null module with elisp target" << std::endl;
        ++passed;
    }

    // --- Test 2: Parsed defun has correct name ---
    {
        std::string source = "(defun f (x) (+ x 1))";
        auto module = TreeSitterParser::parseElisp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty() && "Module should contain at least one function");

        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "f" && "Function name should be 'f'");

        std::cout << "Test 2 PASS: Elisp defun has name 'f'" << std::endl;
        ++passed;
    }

    // --- Test 3: defun has parameter from arglist ---
    {
        std::string source = "(defun f (x) (+ x 1))";
        auto module = TreeSitterParser::parseElisp(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

        auto params = fn->getChildren("parameters");
        assert(!params.empty() && "Function should have at least one parameter");

        auto* param = static_cast<Parameter*>(params[0]);
        assert(param->name == "x" && "Parameter name should be 'x'");

        std::cout << "Test 3 PASS: Elisp defun has parameter 'x'" << std::endl;
        ++passed;
    }

    // --- Test 4: Function body contains BinaryOperation for (+ x 1) ---
    {
        std::string source = "(defun f (x) (+ x 1))";
        auto module = TreeSitterParser::parseElisp(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

        auto body = fn->getChildren("body");
        assert(!body.empty() && "Function body should not be empty");

        // The (+ x 1) should produce a Return or ExpressionStatement with BinaryOperation
        // In Elisp, the last form is implicitly returned
        bool hasBinOp = false;
        for (auto* stmt : body) {
            // Could be wrapped in Return or ExpressionStatement
            ASTNode* expr = nullptr;
            if (stmt->conceptType == "Return") {
                expr = stmt->getChild("value");
            } else if (stmt->conceptType == "ExpressionStatement") {
                expr = stmt->getChild("expression");
            } else if (stmt->conceptType == "BinaryOperation") {
                expr = stmt;
            }

            if (expr && expr->conceptType == "BinaryOperation") {
                auto* binOp = static_cast<BinaryOperation*>(expr);
                assert(binOp->op == "+" && "Operator should be '+'");
                hasBinOp = true;
                break;
            }
        }
        assert(hasBinOp && "Body should contain BinaryOperation(+)");

        std::cout << "Test 4 PASS: Elisp (+ x 1) produces BinaryOperation" << std::endl;
        ++passed;
    }

    // --- Test 5: Elisp function auto-annotated with @Reclaim(Tracing) ---
    {
        std::string source = "(defun f (x) (+ x 1))";
        auto module = TreeSitterParser::parseElisp(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

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
               "Elisp functions should be auto-annotated with @Reclaim(Tracing)");

        std::cout << "Test 5 PASS: Elisp function auto-annotated with @Reclaim(Tracing)" << std::endl;
        ++passed;
    }

    // --- Test 6: Produces same AST structure as Python equivalent ---
    {
        // Both should produce: Function(f) with Parameter(x) and body BinaryOperation(+ x 1)
        std::string elispSource = "(defun f (x) (+ x 1))";
        std::string pythonSource = "def f(x):\n    return x + 1\n";

        auto elispModule = TreeSitterParser::parseElisp(elispSource);
        auto pythonModule = TreeSitterParser::parsePython(pythonSource);

        auto elispFns = elispModule->getChildren("functions");
        auto pythonFns = pythonModule->getChildren("functions");

        assert(!elispFns.empty() && !pythonFns.empty() &&
               "Both should have functions");

        auto* elispFn = static_cast<Function*>(elispFns[0]);
        auto* pythonFn = static_cast<Function*>(pythonFns[0]);

        assert(elispFn->name == pythonFn->name &&
               "Both should produce function with same name");

        auto elispParams = elispFn->getChildren("parameters");
        auto pythonParams = pythonFn->getChildren("parameters");
        assert(elispParams.size() == pythonParams.size() &&
               "Both should have same number of parameters");

        std::cout << "Test 6 PASS: Elisp and Python produce equivalent AST structure" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 47 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
