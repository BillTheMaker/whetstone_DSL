// Step 41 TDD Test: C++ expression generation
//
// Tests that CppGenerator produces correct C++ for:
// 1. BinaryOperation → "left op right" with parentheses for nested ops
// 2. UnaryOperation → "op operand" (e.g., "-x", "!flag")
// 3. VariableReference → variable name
// 4. Literals → C++ equivalents (42, 3.14, "hello", true/false, nullptr)
// 5. FunctionCall → "name(args)"
// 6. Nested expressions with correct parenthesization
//
// Will fail to compile until CppGenerator is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include "ast/Generator.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Simple BinaryOperation ---
    {
        BinaryOperation binOp("b1", "+");
        IntegerLiteral* left = new IntegerLiteral("i1", 3);
        IntegerLiteral* right = new IntegerLiteral("i2", 4);
        binOp.setChild("left", left);
        binOp.setChild("right", right);

        CppGenerator gen;
        std::string output = gen.generate(&binOp);

        assert(contains(output, "3") && "Should contain left operand");
        assert(contains(output, "+") && "Should contain operator");
        assert(contains(output, "4") && "Should contain right operand");

        std::cout << "Test 1 PASS: BinaryOperation generates 'left op right'" << std::endl;
        ++passed;

        delete right;
        delete left;
    }

    // --- Test 2: Nested BinaryOperation gets parenthesized ---
    {
        // (a + b) * c
        BinaryOperation* outerOp = new BinaryOperation("b1", "*");

        BinaryOperation* innerOp = new BinaryOperation("b2", "+");
        VariableReference* a = new VariableReference("v1", "a");
        VariableReference* b = new VariableReference("v2", "b");
        innerOp->setChild("left", a);
        innerOp->setChild("right", b);

        VariableReference* c = new VariableReference("v3", "c");

        outerOp->setChild("left", innerOp);
        outerOp->setChild("right", c);

        CppGenerator gen;
        std::string output = gen.generate(outerOp);

        assert(contains(output, "a") && "Should contain 'a'");
        assert(contains(output, "b") && "Should contain 'b'");
        assert(contains(output, "c") && "Should contain 'c'");
        assert(contains(output, "+") && "Should contain '+'");
        assert(contains(output, "*") && "Should contain '*'");
        // The inner addition should be parenthesized to preserve precedence
        assert(contains(output, "(") && "Nested expression should have parentheses");

        std::cout << "Test 2 PASS: Nested BinaryOperation parenthesized correctly" << std::endl;
        ++passed;

        delete c;
        delete b;
        delete a;
        delete innerOp;
        delete outerOp;
    }

    // --- Test 3: UnaryOperation ---
    {
        UnaryOperation unOp;
        unOp.op = "-";
        VariableReference* operand = new VariableReference("v1", "x");
        unOp.setChild("operand", operand);

        CppGenerator gen;
        std::string output = gen.generate(&unOp);

        assert(contains(output, "-") && "Should contain negation operator");
        assert(contains(output, "x") && "Should contain operand");

        std::cout << "Test 3 PASS: UnaryOperation generates 'op operand'" << std::endl;
        ++passed;

        delete operand;
    }

    // --- Test 4: Integer literal ---
    {
        IntegerLiteral lit("i1", 42);
        CppGenerator gen;
        std::string output = gen.generate(&lit);
        assert(contains(output, "42") && "Should contain integer value");

        std::cout << "Test 4 PASS: IntegerLiteral generates '42'" << std::endl;
        ++passed;
    }

    // --- Test 5: Float literal ---
    {
        FloatLiteral lit("f1", "3.14");
        CppGenerator gen;
        std::string output = gen.generate(&lit);
        assert(contains(output, "3.14") && "Should contain float value");

        std::cout << "Test 5 PASS: FloatLiteral generates '3.14'" << std::endl;
        ++passed;
    }

    // --- Test 6: String literal with quotes ---
    {
        StringLiteral lit("s1", "hello");
        CppGenerator gen;
        std::string output = gen.generate(&lit);
        assert(contains(output, "\"hello\"") && "Should contain quoted string");

        std::cout << "Test 6 PASS: StringLiteral generates '\"hello\"'" << std::endl;
        ++passed;
    }

    // --- Test 7: Boolean literals → C++ true/false ---
    {
        BooleanLiteral trueLit("b1", true);
        BooleanLiteral falseLit("b2", false);
        CppGenerator gen;

        std::string trueOut = gen.generate(&trueLit);
        std::string falseOut = gen.generate(&falseLit);

        assert(contains(trueOut, "true") && "True should generate 'true'");
        assert(contains(falseOut, "false") && "False should generate 'false'");

        std::cout << "Test 7 PASS: BooleanLiterals generate 'true'/'false'" << std::endl;
        ++passed;
    }

    // --- Test 8: Null literal → nullptr ---
    {
        NullLiteral lit;
        CppGenerator gen;
        std::string output = gen.generate(&lit);
        assert(contains(output, "nullptr") && "Null should generate 'nullptr' in C++");

        std::cout << "Test 8 PASS: NullLiteral generates 'nullptr'" << std::endl;
        ++passed;
    }

    // --- Test 9: FunctionCall with arguments ---
    {
        FunctionCall call;
        call.functionName = "calculate";
        IntegerLiteral* arg1 = new IntegerLiteral("i1", 5);
        VariableReference* arg2 = new VariableReference("v1", "x");
        call.addChild("arguments", arg1);
        call.addChild("arguments", arg2);

        CppGenerator gen;
        std::string output = gen.generate(&call);

        assert(contains(output, "calculate") && "Should contain function name");
        assert(contains(output, "(") && "Should contain opening paren");
        assert(contains(output, ")") && "Should contain closing paren");
        assert(contains(output, "5") && "Should contain first argument");
        assert(contains(output, "x") && "Should contain second argument");

        std::cout << "Test 9 PASS: FunctionCall generates 'name(args)'" << std::endl;
        ++passed;

        delete arg2;
        delete arg1;
    }

    // --- Test 10: VariableReference ---
    {
        VariableReference ref("v1", "myVariable");
        CppGenerator gen;
        std::string output = gen.generate(&ref);
        assert(contains(output, "myVariable") && "Should contain variable name");

        std::cout << "Test 10 PASS: VariableReference generates variable name" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 41 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
