// Step 366: WAT Generator (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "Pipeline.h"
#include "ast/WatGenerator.h"
#include "ast/WatParser.h"

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

    // Test 1: function generation
    {
        WatGenerator gen;
        Function fn("f1", "add");
        auto* p1 = new Parameter("p1", "x");
        p1->setChild("type", new PrimitiveType("t1", "int"));
        fn.addChild("parameters", p1);
        fn.setChild("returnType", new PrimitiveType("rt1", "int"));
        auto out = gen.generate(&fn);
        assert(out.find("(func $add") != std::string::npos);
        assert(out.find("(param $x i32)") != std::string::npos);
        assert(out.find("(result i32)") != std::string::npos);
        std::cout << "Test 1 PASSED: function generation\n";
        passed++;
    }

    // Test 2: S-expression formatting for module
    {
        WatGenerator gen;
        Module mod("m1", "wat_mod", "wat");
        auto out = gen.generate(&mod);
        assert(out.find("(module") == 0);
        assert(out.back() == ')');
        std::cout << "Test 2 PASSED: module s-expression formatting\n";
        passed++;
    }

    // Test 3: type mapping
    {
        WatGenerator gen;
        Function fn("f1", "types");
        auto* p = new Parameter("p1", "a");
        p->setChild("type", new PrimitiveType("t1", "double"));
        fn.addChild("parameters", p);
        fn.setChild("returnType", new PrimitiveType("rt", "long"));
        auto out = gen.generate(&fn);
        assert(out.find("f64") != std::string::npos);
        assert(out.find("i64") != std::string::npos);
        std::cout << "Test 3 PASSED: type mapping\n";
        passed++;
    }

    // Test 4: arithmetic to stack instruction
    {
        WatGenerator gen;
        BinaryOperation add;
        add.id = "b1";
        add.op = "+";
        add.setChild("left", new VariableReference("vr1", "x"));
        add.setChild("right", new VariableReference("vr2", "y"));
        auto out = gen.generate(&add);
        assert(out.find("i32.add") != std::string::npos);
        assert(out.find("local.get $x") != std::string::npos);
        std::cout << "Test 4 PASSED: arithmetic stack lowering\n";
        passed++;
    }

    // Test 5: if/else to WAT if
    {
        WatGenerator gen;
        IfStatement stmt;
        stmt.id = "if1";
        stmt.setChild("condition", new BooleanLiteral("b1", true));
        stmt.addChild("thenBranch", new IntegerLiteral("i1", 1));
        stmt.addChild("elseBranch", new IntegerLiteral("i2", 0));
        auto out = gen.generate(&stmt);
        assert(out.find("(if") != std::string::npos);
        assert(out.find("(then") != std::string::npos);
        assert(out.find("(else") != std::string::npos);
        std::cout << "Test 5 PASSED: if/else generation\n";
        passed++;
    }

    // Test 6: function call to call instruction
    {
        WatGenerator gen;
        FunctionCall call;
        call.id = "c1";
        call.functionName = "helper";
        auto out = gen.generate(&call);
        assert(out.find("call $helper") != std::string::npos);
        std::cout << "Test 6 PASSED: function call generation\n";
        passed++;
    }

    // Test 7: export declaration
    {
        WatGenerator gen;
        PragmaDirective exp("p1", "export");
        auto out = gen.generate(&exp);
        assert(out.find("(export") != std::string::npos);
        std::cout << "Test 7 PASSED: export generation\n";
        passed++;
    }

    // Test 8: import declaration
    {
        WatGenerator gen;
        Module mod("m1", "imports", "wat");
        mod.addChild("imports", new Import("i1", "env.print", "runtime", ""));
        auto out = gen.generate(&mod);
        assert(out.find("(import") != std::string::npos);
        assert(out.find("\"env\"") != std::string::npos);
        std::cout << "Test 8 PASSED: import generation\n";
        passed++;
    }

    // Test 9: Semanno comments with ;; prefix
    {
        WatGenerator gen;
        Function fn("f1", "owned");
        fn.addChild("annotations", new OwnerAnnotation("a1", "Manual"));
        auto out = gen.generate(&fn);
        assert(out.find(";; @owner(Manual)") != std::string::npos);
        std::cout << "Test 9 PASSED: semanno-style comment prefix\n";
        passed++;
    }

    // Test 10: cross-language pipeline Python -> WAT
    {
        Pipeline p;
        std::string py = "def add(x, y):\n    return x + y\n";
        auto result = p.run(py, "python", "wat");
        assert(result.success);
        assert(result.generatedCode.find("(module") != std::string::npos);
        std::cout << "Test 10 PASSED: Python->WAT pipeline route\n";
        passed++;
    }

    // Test 11: round-trip parse -> generate -> parse
    {
        std::string wat = "(module (func $add (param $x i32) (param $y i32) (result i32) i32.add))";
        auto parsed = WatParser::parseWat(wat);
        WatGenerator gen;
        std::string out = gen.generate(parsed.get());
        auto reparsed = WatParser::parseWat(out);
        assert(!reparsed->getChildren("functions").empty());
        std::cout << "Test 11 PASSED: parse/generate/parse roundtrip\n";
        passed++;
    }

    // Test 12: parentheses balanced in generated module
    {
        WatGenerator gen;
        Module mod("m1", "bal", "wat");
        Function* fn = new Function("f1", "noop");
        mod.addChild("functions", fn);
        auto out = gen.generate(&mod);
        assert(parenBalance(out) == 0);
        std::cout << "Test 12 PASSED: balanced parentheses\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
