// Step 371: Common Lisp Generator (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "Pipeline.h"
#include "ast/CommonLispGenerator.h"
#include "ast/CommonLispParser.h"
#include "ast/ClassDeclaration.h"

int main() {
    int passed = 0;

    // Test 1: defun generation
    {
        CommonLispGenerator gen;
        Function fn("f1", "add");
        fn.addChild("parameters", new Parameter("p1", "x"));
        fn.addChild("parameters", new Parameter("p2", "y"));
        auto* ret = new Return();
        ret->id = "r1";
        ret->setChild("value", new BinaryOperation("b1", "+"));
        fn.addChild("body", ret);
        auto out = gen.generate(&fn);
        assert(out.find("(defun add (x y)") != std::string::npos);
        std::cout << "Test 1 PASSED: defun generation\n";
        passed++;
    }

    // Test 2: defclass output
    {
        CommonLispGenerator gen;
        ClassDeclaration cls("c1", "point");
        cls.addBase("shape");
        cls.addChild("fields", new Variable("v1", "x"));
        cls.addChild("fields", new Variable("v2", "y"));
        auto out = gen.generate(&cls);
        assert(out.find("(defclass point (shape)") != std::string::npos);
        assert(out.find("(x :initarg :x)") != std::string::npos);
        std::cout << "Test 2 PASSED: defclass output\n";
        passed++;
    }

    // Test 3: lambda output
    {
        CommonLispGenerator gen;
        LambdaExpression lambda("l1");
        lambda.addChild("parameters", new Parameter("p1", "x"));
        lambda.addChild("body", new ExpressionStatement());
        auto out = gen.generate(&lambda);
        assert(out.find("(lambda (x)") != std::string::npos);
        std::cout << "Test 3 PASSED: lambda output\n";
        passed++;
    }

    // Test 4: let bindings output from block variables
    {
        CommonLispGenerator gen;
        Block block;
        block.id = "blk1";
        auto* x = new Variable("v1", "x");
        x->setChild("value", new IntegerLiteral("i1", 1));
        auto* y = new Variable("v2", "y");
        y->setChild("value", new IntegerLiteral("i2", 2));
        block.addChild("statements", x);
        block.addChild("statements", y);
        block.addChild("statements", new ExpressionStatement());
        auto out = gen.generate(&block);
        assert(out.find("(let ((x 1) (y 2))") != std::string::npos);
        std::cout << "Test 4 PASSED: let bindings output\n";
        passed++;
    }

    // Test 5: S-expression formatting and balanced parens
    {
        CommonLispGenerator gen;
        Module mod("m1", "demo", "common-lisp");
        mod.addChild("functions", new Function("f1", "noop"));
        auto out = gen.generate(&mod);
        int depth = 0;
        for (char c : out) {
            if (c == '(') ++depth;
            if (c == ')') --depth;
        }
        assert(depth == 0);
        assert(out.find("(defun noop") != std::string::npos);
        std::cout << "Test 5 PASSED: s-expression formatting\n";
        passed++;
    }

    // Test 6: type declarations from parameter types
    {
        CommonLispGenerator gen;
        Function fn("f1", "typed");
        auto* p = new Parameter("p1", "x");
        p->setChild("type", new PrimitiveType("t1", "int"));
        fn.addChild("parameters", p);
        auto out = gen.generate(&fn);
        assert(out.find("(declare (type fixnum x))") != std::string::npos);
        std::cout << "Test 6 PASSED: type declaration emission\n";
        passed++;
    }

    // Test 7: Python -> Common Lisp pipeline
    {
        Pipeline p;
        std::string src = "def add(x, y):\n    return x + y\n";
        auto result = p.run(src, "python", "common-lisp");
        assert(result.success);
        assert(result.generatedCode.find("(defun") != std::string::npos);
        std::cout << "Test 7 PASSED: Python -> Common Lisp pipeline\n";
        passed++;
    }

    // Test 8: Java -> Common Lisp pipeline
    {
        Pipeline p;
        std::string src = "class A { int id(){ return 1; } }";
        auto result = p.run(src, "java", "common-lisp");
        assert(result.success);
        assert(result.generatedCode.find("(defun") != std::string::npos ||
               result.generatedCode.find("(defclass") != std::string::npos);
        std::cout << "Test 8 PASSED: Java -> Common Lisp pipeline\n";
        passed++;
    }

    // Test 9: semanno comments with ;; prefix
    {
        CommonLispGenerator gen;
        Function fn("f1", "owned");
        fn.addChild("annotations", new OwnerAnnotation("a1", "Shared_GC"));
        auto out = gen.generate(&fn);
        assert(out.find(";; @owner(Shared_GC)") != std::string::npos);
        std::cout << "Test 9 PASSED: Semanno comment prefix\n";
        passed++;
    }

    // Test 10: parse -> generate -> parse roundtrip
    {
        std::string source = "(defun round (x) (+ x 1))";
        auto parsed = CommonLispParser::parseCommonLisp(source);
        CommonLispGenerator gen;
        auto emitted = gen.generate(parsed.get());
        auto reparsed = CommonLispParser::parseCommonLisp(emitted);
        assert(!reparsed->getChildren("functions").empty());
        std::cout << "Test 10 PASSED: roundtrip parse/generate/parse\n";
        passed++;
    }

    // Test 11: CLOS method dispatch output
    {
        CommonLispGenerator gen;
        MethodDeclaration method("m1", "area");
        auto* p = new Parameter("p1", "obj");
        p->setChild("type", new CustomType("t1", "rectangle"));
        method.addChild("parameters", p);
        auto out = gen.generate(&method);
        assert(out.find("(defmethod area ((obj rectangle))") != std::string::npos);
        std::cout << "Test 11 PASSED: CLOS method output\n";
        passed++;
    }

    // Test 12: condition-system wrapper from exception annotation
    {
        CommonLispGenerator gen;
        Function fn("f1", "safe-op");
        auto* ex = new ExceptionAnnotation();
        ex->id = "a1";
        ex->style = "checked";
        fn.addChild("annotations", ex);
        fn.addChild("body", new ExpressionStatement());
        auto out = gen.generate(&fn);
        assert(out.find("handler-case") != std::string::npos);
        assert(out.find("(error (e) e)") != std::string::npos);
        std::cout << "Test 12 PASSED: condition system wrapper\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
