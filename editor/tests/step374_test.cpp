// Step 374: Scheme Parser (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "Pipeline.h"
#include "ast/SchemeParser.h"

static bool hasBodyNode(const Function* fn, const std::string& conceptType) {
    for (auto* stmt : fn->getChildren("body")) {
        if (stmt->conceptType == conceptType) return true;
        if (stmt->conceptType == "ExpressionStatement") {
            ASTNode* expr = stmt->getChild("expression");
            if (expr && expr->conceptType == conceptType) return true;
        }
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: define function parsing
    {
        auto mod = SchemeParser::parseScheme("(define (add x y) (+ x y))");
        assert(mod->targetLanguage == "scheme");
        assert(mod->getChildren("functions").size() == 1);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(fn->name == "add");
        assert(fn->getChildren("parameters").size() == 2);
        std::cout << "Test 1 PASSED: define function parsing\n";
        passed++;
    }

    // Test 2: define variable parsing
    {
        auto mod = SchemeParser::parseScheme("(define answer 42)");
        assert(mod->getChildren("variables").size() == 1);
        auto* var = static_cast<Variable*>(mod->getChildren("variables")[0]);
        assert(var->name == "answer");
        assert(var->getChild("value") != nullptr);
        std::cout << "Test 2 PASSED: define variable parsing\n";
        passed++;
    }

    // Test 3: lambda parsing
    {
        auto mod = SchemeParser::parseScheme("(define (mk) (lambda (x) (+ x 1)))");
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "LambdaExpression"));
        std::cout << "Test 3 PASSED: lambda parsing\n";
        passed++;
    }

    // Test 4: let / let* / letrec parsing
    {
        std::string src =
            "(define (a) (let ((x 1)) x))\n"
            "(define (b) (let* ((x 1) (y 2)) (+ x y)))\n"
            "(define (c) (letrec ((f (lambda (n) n))) (f 1)))";
        auto mod = SchemeParser::parseScheme(src);
        auto* fa = static_cast<Function*>(mod->getChildren("functions")[0]);
        auto* fb = static_cast<Function*>(mod->getChildren("functions")[1]);
        auto* fc = static_cast<Function*>(mod->getChildren("functions")[2]);
        assert(hasBodyNode(fa, "Block"));
        assert(hasBodyNode(fb, "Block"));
        assert(hasBodyNode(fc, "Block"));
        std::cout << "Test 4 PASSED: let variants parsing\n";
        passed++;
    }

    // Test 5: if and cond parsing
    {
        std::string src =
            "(define (s x) (if x 1 0))\n"
            "(define (c x) (cond ((> x 0) 1) ((< x 0) -1) (else 0)))";
        auto mod = SchemeParser::parseScheme(src);
        auto* f1 = static_cast<Function*>(mod->getChildren("functions")[0]);
        auto* f2 = static_cast<Function*>(mod->getChildren("functions")[1]);
        assert(hasBodyNode(f1, "IfStatement"));
        assert(hasBodyNode(f2, "IfStatement"));
        std::cout << "Test 5 PASSED: if/cond parsing\n";
        passed++;
    }

    // Test 6: do loop parsing
    {
        std::string src = "(define (loopn n) (do ((i 0 (+ i 1))) ((> i n) i) (display i)))";
        auto mod = SchemeParser::parseScheme(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "ForLoop"));
        std::cout << "Test 6 PASSED: do loop parsing\n";
        passed++;
    }

    // Test 7: call/cc continuation annotation
    {
        std::string src =
            "(define (cc f) (call-with-current-continuation f))\n"
            "(define (cc2 f) (call/cc f))";
        auto mod = SchemeParser::parseScheme(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        bool hasContinuation = false;
        for (auto* stmt : fn->getChildren("body")) {
            ASTNode* expr = stmt->getChild("expression");
            if (!expr || expr->conceptType != "FunctionCall") continue;
            for (auto* anno : expr->getChildren("annotations")) {
                if (anno->conceptType == "ExecAnnotation") {
                    auto* ex = static_cast<ExecAnnotation*>(anno);
                    if (ex->mode == "continuation") hasContinuation = true;
                }
            }
        }
        assert(hasContinuation);
        std::cout << "Test 7 PASSED: call/cc continuation annotation\n";
        passed++;
    }

    // Test 8: define-syntax -> macro + hygienic meta
    {
        std::string src = "(define-syntax when1 (syntax-rules () ((_ t b) (if t b #f))))";
        auto mod = SchemeParser::parseScheme(src);
        bool foundMacro = false;
        bool hygienic = false;
        for (auto* stmt : mod->getChildren("statements")) {
            if (stmt->conceptType != "MacroDefinition") continue;
            foundMacro = true;
            for (auto* anno : stmt->getChildren("annotations")) {
                if (anno->conceptType == "MetaAnnotation") {
                    auto* m = static_cast<MetaAnnotation*>(anno);
                    if (m->state == "hygienic") hygienic = true;
                }
            }
        }
        assert(foundMacro && hygienic);
        std::cout << "Test 8 PASSED: define-syntax hygienic macro mapping\n";
        passed++;
    }

    // Test 9: values multi-return mapping
    {
        std::string src = "(define (pair x) (values x (+ x 1)))";
        auto mod = SchemeParser::parseScheme(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        bool hasContract = false;
        for (auto* stmt : fn->getChildren("body")) {
            ASTNode* expr = stmt->getChild("expression");
            if (!expr || expr->conceptType != "FunctionCall") continue;
            for (auto* anno : expr->getChildren("annotations")) {
                if (anno->conceptType == "ContractAnnotation") hasContract = true;
            }
        }
        assert(hasContract);
        std::cout << "Test 9 PASSED: values -> Contract annotation\n";
        passed++;
    }

    // Test 10: begin form maps to sequence block
    {
        std::string src = "(define (f) (begin (display 1) (display 2) 3))";
        auto mod = SchemeParser::parseScheme(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "Block"));
        std::cout << "Test 10 PASSED: begin -> block sequence\n";
        passed++;
    }

    // Test 11: deep nested s-expressions
    {
        std::string src = "(define (deep x) (a (b (c (d (e (f (g x))))))))";
        auto mod = SchemeParser::parseScheme(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "FunctionCall"));
        std::cout << "Test 11 PASSED: deep nesting parse\n";
        passed++;
    }

    // Test 12: mixed source + pipeline route + diagnostics
    {
        std::string src =
            "(define x 1)\n"
            "(define (id y) y)\n"
            "(define-syntax m (syntax-rules () ((_ z) z)))\n";
        auto mod = SchemeParser::parseScheme(src);
        assert(mod->getChildren("variables").size() == 1);
        assert(mod->getChildren("functions").size() == 1);
        bool macroFound = false;
        for (auto* stmt : mod->getChildren("statements")) {
            if (stmt->conceptType == "MacroDefinition") macroFound = true;
        }
        assert(macroFound);

        Pipeline p;
        std::vector<ParseDiagnostic> diags;
        auto parsedScheme = p.parse("(define (f x) x)", "scheme", diags);
        auto parsedScm = p.parse("(define (f x) x)", "scm", diags);
        assert(parsedScheme != nullptr && parsedScm != nullptr);

        auto malformed = SchemeParser::parseSchemeWithDiagnostics("(define (f x) (+ x 1)");
        assert(!malformed.diagnostics.empty());
        std::cout << "Test 12 PASSED: mixed forms + pipeline route + diagnostics\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
