// Step 370: Common Lisp Parser (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "Pipeline.h"
#include "ast/CommonLispParser.h"
#include "ast/ClassDeclaration.h"

static bool hasBodyNode(const Function* fn, const std::string& conceptType) {
    for (auto* node : fn->getChildren("body")) {
        if (node->conceptType == conceptType) return true;
        if (node->conceptType == "ExpressionStatement") {
            ASTNode* expr = node->getChild("expression");
            if (expr && expr->conceptType == conceptType) return true;
        }
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: defun parsing
    {
        std::string src = "(defun add (x y) (+ x y))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        assert(mod->targetLanguage == "common-lisp");
        assert(mod->getChildren("functions").size() == 1);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(fn->name == "add");
        assert(fn->getChildren("parameters").size() == 2);
        std::cout << "Test 1 PASSED: defun parsing\n";
        passed++;
    }

    // Test 2: defclass parsing
    {
        std::string src = "(defclass point (shape) ((x) (y)))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        assert(mod->getChildren("classes").size() == 1);
        auto* cls = static_cast<ClassDeclaration*>(mod->getChildren("classes")[0]);
        assert(cls->name == "point");
        assert(cls->getChildren("fields").size() == 2);
        assert(!cls->getBases().empty() && cls->getBases()[0].name == "shape");
        std::cout << "Test 2 PASSED: defclass parsing\n";
        passed++;
    }

    // Test 3: lambda parsing
    {
        std::string src = "(defun mk () (lambda (x) (+ x 1)))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "LambdaExpression"));
        std::cout << "Test 3 PASSED: lambda parsing\n";
        passed++;
    }

    // Test 4: let bindings
    {
        std::string src = "(defun f () (let ((x 1) (y 2)) (+ x y)))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        bool foundBlock = false;
        bool foundVar = false;
        for (auto* stmt : fn->getChildren("body")) {
            ASTNode* expr = stmt->getChild("expression");
            if (!expr || expr->conceptType != "Block") continue;
            foundBlock = true;
            for (auto* child : expr->getChildren("statements")) {
                if (child->conceptType == "Variable") foundVar = true;
            }
        }
        assert(foundBlock && foundVar);
        std::cout << "Test 4 PASSED: let bindings\n";
        passed++;
    }

    // Test 5: if and cond parsing
    {
        std::string src =
            "(defun choose (x) (if x 1 0))\n"
            "(defun choose2 (x) (cond ((> x 0) 1) ((< x 0) -1) (t 0)))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        auto* fn1 = static_cast<Function*>(mod->getChildren("functions")[0]);
        auto* fn2 = static_cast<Function*>(mod->getChildren("functions")[1]);
        assert(hasBodyNode(fn1, "IfStatement"));
        assert(hasBodyNode(fn2, "IfStatement"));
        std::cout << "Test 5 PASSED: if/cond parsing\n";
        passed++;
    }

    // Test 6: loop variants
    {
        std::string src =
            "(defun a () (loop for i from 0 to 3 do (print i)))\n"
            "(defun b () (do ((i 0 (+ i 1))) ((> i 3) i)))\n"
            "(defun c () (dotimes (i 3) (print i)))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        auto* fn1 = static_cast<Function*>(mod->getChildren("functions")[0]);
        auto* fn2 = static_cast<Function*>(mod->getChildren("functions")[1]);
        auto* fn3 = static_cast<Function*>(mod->getChildren("functions")[2]);
        assert(hasBodyNode(fn1, "ForLoop"));
        assert(hasBodyNode(fn2, "ForLoop"));
        assert(hasBodyNode(fn3, "ForLoop"));
        std::cout << "Test 6 PASSED: loop/do/dotimes parsing\n";
        passed++;
    }

    // Test 7: defmacro parsing
    {
        std::string src = "(defmacro when1 (test body) `(if ,test ,body nil))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        bool foundMacro = false;
        for (auto* stmt : mod->getChildren("statements")) {
            if (stmt->conceptType == "MacroDefinition") foundMacro = true;
        }
        assert(foundMacro);
        std::cout << "Test 7 PASSED: defmacro parsing\n";
        passed++;
    }

    // Test 8: quote and shorthand quote annotation
    {
        std::string src = "(defun q () (quote x) 'y)";
        auto mod = CommonLispParser::parseCommonLisp(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        bool hasQuoted = false;
        for (auto* stmt : fn->getChildren("body")) {
            ASTNode* expr = stmt->getChild("expression");
            if (!expr) continue;
            for (auto* anno : expr->getChildren("annotations")) {
                if (anno->conceptType == "MetaAnnotation") hasQuoted = true;
            }
        }
        assert(hasQuoted);
        std::cout << "Test 8 PASSED: quote annotation\n";
        passed++;
    }

    // Test 9: earmuff dynamic variable detection
    {
        std::string src = "(defvar *runtime-mode* 1)";
        auto mod = CommonLispParser::parseCommonLisp(src);
        assert(mod->getChildren("variables").size() == 1);
        auto* var = static_cast<Variable*>(mod->getChildren("variables")[0]);
        bool dynamic = false;
        for (auto* anno : var->getChildren("annotations")) {
            if (anno->conceptType == "BindingAnnotation") {
                auto* b = static_cast<BindingAnnotation*>(anno);
                if (b->time == "dynamic") dynamic = true;
            }
        }
        assert(dynamic);
        std::cout << "Test 9 PASSED: earmuff dynamic variable detection\n";
        passed++;
    }

    // Test 10: deep nested s-expressions
    {
        std::string src = "(defun deep (x) (a (b (c (d (e (f (g x))))))))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
        assert(hasBodyNode(fn, "FunctionCall"));
        std::cout << "Test 10 PASSED: deep nesting parse\n";
        passed++;
    }

    // Test 11: mixed definitions in one file
    {
        std::string src =
            "(defclass user () ((name) (age)))\n"
            "(defvar +limit+ 32)\n"
            "(defmacro with-u (u body) `(let ((x ,u)) ,body))\n"
            "(cl:defun my-package:run (n) n)";
        auto mod = CommonLispParser::parseCommonLisp(src);
        assert(mod->getChildren("classes").size() == 1);
        assert(mod->getChildren("variables").size() == 1);
        assert(mod->getChildren("functions").size() == 1);
        assert(static_cast<Function*>(mod->getChildren("functions")[0])->name == "my-package:run");
        bool macroFound = false;
        for (auto* stmt : mod->getChildren("statements")) {
            if (stmt->conceptType == "MacroDefinition") macroFound = true;
        }
        assert(macroFound);
        std::cout << "Test 11 PASSED: mixed definitions and package prefixes\n";
        passed++;
    }

    // Test 12: CLOS method + declare type + pipeline route aliases
    {
        std::string src =
            "(defmethod area ((obj rectangle))\n"
            "  (declare (type fixnum obj))\n"
            "  (values 1 2))";
        auto mod = CommonLispParser::parseCommonLisp(src);
        assert(mod->getChildren("functions").size() == 1);
        auto* method = static_cast<MethodDeclaration*>(mod->getChildren("functions")[0]);
        assert(method->conceptType == "MethodDeclaration");
        assert(method->className == "rectangle");
        auto* param = static_cast<Parameter*>(method->getChildren("parameters")[0]);
        assert(param->getChild("type") != nullptr);

        Pipeline p;
        std::vector<ParseDiagnostic> diags;
        auto parsed1 = p.parse("(defun f (x) x)", "common-lisp", diags);
        auto parsed2 = p.parse("(defun f (x) x)", "lisp", diags);
        assert(parsed1 != nullptr && parsed2 != nullptr);
        assert(parsed1->targetLanguage == "common-lisp");
        std::cout << "Test 12 PASSED: defmethod + declare + pipeline aliases\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
