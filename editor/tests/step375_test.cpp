// Step 375: Scheme Generator (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "Pipeline.h"
#include "ast/SchemeGenerator.h"
#include "ast/SchemeParser.h"

static int parenBalance(const std::string& s) {
    int d = 0;
    for (char c : s) {
        if (c == '(') ++d;
        if (c == ')') --d;
    }
    return d;
}

int main() {
    int passed = 0;

    // Test 1: define function generation
    {
        SchemeGenerator gen;
        Function fn("f1", "add");
        fn.addChild("parameters", new Parameter("p1", "x"));
        fn.addChild("parameters", new Parameter("p2", "y"));
        auto out = gen.generate(&fn);
        assert(out.find("(define (add x y)") != std::string::npos);
        std::cout << "Test 1 PASSED: define function generation\n";
        passed++;
    }

    // Test 2: lambda output
    {
        SchemeGenerator gen;
        LambdaExpression lambda("l1");
        lambda.addChild("parameters", new Parameter("p1", "x"));
        auto out = gen.generate(&lambda);
        assert(out.find("(lambda (x)") != std::string::npos);
        std::cout << "Test 2 PASSED: lambda output\n";
        passed++;
    }

    // Test 3: let bindings output
    {
        SchemeGenerator gen;
        Block block;
        block.id = "b1";
        auto* v = new Variable("v1", "x");
        v->setChild("value", new IntegerLiteral("i1", 1));
        block.addChild("statements", v);
        block.addChild("statements", new ExpressionStatement());
        auto out = gen.generate(&block);
        assert(out.find("(let ((x 1))") != std::string::npos);
        std::cout << "Test 3 PASSED: let binding output\n";
        passed++;
    }

    // Test 4: booleans map to #t / #f
    {
        SchemeGenerator gen;
        auto t = gen.generate(new BooleanLiteral("b1", true));
        auto f = gen.generate(new BooleanLiteral("b2", false));
        assert(t == "#t");
        assert(f == "#f");
        std::cout << "Test 4 PASSED: boolean mapping\n";
        passed++;
    }

    // Test 5: S-expression formatting / balanced parens
    {
        SchemeGenerator gen;
        Module mod("m1", "scheme_mod", "scheme");
        mod.addChild("functions", new Function("f1", "noop"));
        auto out = gen.generate(&mod);
        assert(parenBalance(out) == 0);
        assert(out.find("(define (noop)") != std::string::npos);
        std::cout << "Test 5 PASSED: balanced S-expression output\n";
        passed++;
    }

    // Test 6: cross-language Python -> Scheme
    {
        Pipeline p;
        std::string py = "def add(x, y):\n    return x + y\n";
        auto res = p.run(py, "python", "scheme");
        assert(res.success);
        assert(res.generatedCode.find("(define (add x y)") != std::string::npos);
        std::cout << "Test 6 PASSED: Python -> Scheme pipeline\n";
        passed++;
    }

    // Test 7: cross-language Common Lisp -> Scheme
    {
        Pipeline p;
        std::string cl = "(defun id (x) x)";
        auto res = p.run(cl, "common-lisp", "scheme");
        assert(res.success);
        assert(res.generatedCode.find("(define (id x)") != std::string::npos);
        std::cout << "Test 7 PASSED: Common Lisp -> Scheme pipeline\n";
        passed++;
    }

    // Test 8: cross-language Scheme -> Python
    {
        Pipeline p;
        std::string scm = "(define (id x) x)";
        auto res = p.run(scm, "scheme", "python");
        assert(res.success);
        assert(res.generatedCode.find("def id(x)") != std::string::npos);
        std::cout << "Test 8 PASSED: Scheme -> Python pipeline\n";
        passed++;
    }

    // Test 9: semanno-style comments use '; ' prefix
    {
        SchemeGenerator gen;
        Function fn("f1", "owned");
        fn.addChild("annotations", new OwnerAnnotation("a1", "Shared_GC"));
        auto out = gen.generate(&fn);
        assert(out.find("; @owner(strategy=Shared_GC)") != std::string::npos);
        std::cout << "Test 9 PASSED: semanno comment prefix\n";
        passed++;
    }

    // Test 10: parse -> generate -> parse roundtrip
    {
        std::string src = "(define (id x) x)";
        auto parsed = SchemeParser::parseScheme(src);
        SchemeGenerator gen;
        auto emitted = gen.generate(parsed.get());
        auto reparsed = SchemeParser::parseScheme(emitted);
        assert(!reparsed->getChildren("functions").empty());
        std::cout << "Test 10 PASSED: parse/generate/parse roundtrip\n";
        passed++;
    }

    // Test 11: tail-call annotation preserved in generated comments
    {
        SchemeGenerator gen;
        Function fn("f1", "loopf");
        auto* t = new TailCallAnnotation();
        t->id = "a1";
        fn.addChild("annotations", t);
        auto out = gen.generate(&fn);
        assert(out.find("; @tailcall()") != std::string::npos);
        std::cout << "Test 11 PASSED: tail-call annotation comment\n";
        passed++;
    }

    // Test 12: continuation annotation generates call/cc form
    {
        SchemeGenerator gen;
        FunctionCall call;
        call.id = "c1";
        call.functionName = "handler";
        auto* ex = new ExecAnnotation();
        ex->id = "a1";
        ex->mode = "continuation";
        call.addChild("annotations", ex);
        call.addChild("arguments", new VariableReference("r1", "k"));
        auto out = gen.generate(&call);
        assert(out.find("(call/cc") != std::string::npos);
        std::cout << "Test 12 PASSED: continuation annotation generation\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
