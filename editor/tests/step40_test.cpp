// Step 40 TDD Test: C++ statement generation
//
// Tests that CppGenerator produces correct C++ for:
// 1. Assignment → "target = value;"  (with semicolon)
// 2. Return → "return value;"  (with semicolon)
// 3. IfStatement → "if (...) { ... } else { ... }"  (braces, no colons)
// 4. WhileLoop → "while (...) { ... }"
// 5. ForLoop → range-based "for (auto iter : iterable) { ... }"
//
// Will fail to compile until CppGenerator is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include "ast/Generator.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

static bool notContains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) == std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Assignment with semicolon ---
    {
        Assignment assign;
        VariableReference* target = new VariableReference("v1", "result");
        IntegerLiteral* value = new IntegerLiteral("i1", 42);
        assign.setChild("target", target);
        assign.setChild("value", value);

        CppGenerator gen;
        std::string output = gen.generate(&assign);

        assert(contains(output, "result") && "Assignment should contain target name");
        assert(contains(output, "=") && "Assignment should contain '='");
        assert(contains(output, "42") && "Assignment should contain value");
        assert(contains(output, ";") && "C++ assignment must end with semicolon");

        std::cout << "Test 1 PASS: Assignment generates 'target = value;'" << std::endl;
        ++passed;

        delete value;
        delete target;
    }

    // --- Test 2: Return statement with semicolon ---
    {
        Return ret;
        IntegerLiteral* val = new IntegerLiteral("i1", 7);
        ret.setChild("value", val);

        CppGenerator gen;
        std::string output = gen.generate(&ret);

        assert(contains(output, "return") && "Return should contain 'return'");
        assert(contains(output, "7") && "Return should contain the value");
        assert(contains(output, ";") && "C++ return must end with semicolon");

        std::cout << "Test 2 PASS: Return generates 'return value;'" << std::endl;
        ++passed;

        delete val;
    }

    // --- Test 3: IfStatement with braces ---
    {
        IfStatement ifStmt;
        BooleanLiteral* cond = new BooleanLiteral("b1", true);
        ifStmt.setChild("condition", cond);

        // Then branch: return 1;
        Return* thenRet = new Return();
        IntegerLiteral* thenVal = new IntegerLiteral("i1", 1);
        thenRet->setChild("value", thenVal);
        ifStmt.addChild("thenBranch", thenRet);

        // Else branch: return 0;
        Return* elseRet = new Return();
        IntegerLiteral* elseVal = new IntegerLiteral("i2", 0);
        elseRet->setChild("value", elseVal);
        ifStmt.addChild("elseBranch", elseRet);

        CppGenerator gen;
        std::string output = gen.generate(&ifStmt);

        assert(contains(output, "if") && "IfStatement should contain 'if'");
        assert(contains(output, "(") && "Condition should be in parentheses");
        assert(contains(output, ")") && "Condition should be in parentheses");
        assert(contains(output, "{") && "C++ if should use braces");
        assert(contains(output, "}") && "C++ if should use braces");
        assert(contains(output, "else") && "Should contain else branch");
        assert(contains(output, "return") && "Should contain return statements");

        std::cout << "Test 3 PASS: IfStatement generates if/else with braces" << std::endl;
        ++passed;

        delete elseVal;
        delete elseRet;
        delete thenVal;
        delete thenRet;
        delete cond;
    }

    // --- Test 4: WhileLoop with braces ---
    {
        WhileLoop loop;
        BooleanLiteral* cond = new BooleanLiteral("b1", true);
        loop.setChild("condition", cond);

        // Body: a simple assignment
        Assignment* assign = new Assignment();
        VariableReference* target = new VariableReference("v1", "count");
        IntegerLiteral* value = new IntegerLiteral("i1", 0);
        assign->setChild("target", target);
        assign->setChild("value", value);
        loop.addChild("body", assign);

        CppGenerator gen;
        std::string output = gen.generate(&loop);

        assert(contains(output, "while") && "WhileLoop should contain 'while'");
        assert(contains(output, "(") && "Condition should be in parentheses");
        assert(contains(output, "{") && "C++ while should use braces");
        assert(contains(output, "}") && "C++ while should use closing brace");

        std::cout << "Test 4 PASS: WhileLoop generates while(...) { ... }" << std::endl;
        ++passed;

        delete value;
        delete target;
        delete assign;
        delete cond;
    }

    // --- Test 5: ForLoop ---
    {
        ForLoop loop;
        loop.iteratorName = "item";
        VariableReference* iterable = new VariableReference("v1", "items");
        loop.setChild("iterable", iterable);

        // Body: expression statement
        ExpressionStatement* exprStmt = new ExpressionStatement();
        FunctionCall* call = new FunctionCall();
        call->functionName = "process";
        VariableReference* arg = new VariableReference("v2", "item");
        call->addChild("arguments", arg);
        exprStmt->setChild("expression", call);
        loop.addChild("body", exprStmt);

        CppGenerator gen;
        std::string output = gen.generate(&loop);

        assert(contains(output, "for") && "ForLoop should contain 'for'");
        assert(contains(output, "(") && "For should have parentheses");
        assert(contains(output, "item") && "For should contain iterator name");
        assert(contains(output, "{") && "C++ for should use braces");
        assert(contains(output, "}") && "C++ for should use closing brace");

        std::cout << "Test 5 PASS: ForLoop generates for loop with braces" << std::endl;
        ++passed;

        delete arg;
        delete call;
        delete exprStmt;
        delete iterable;
    }

    // --- Test 6: No Python-style syntax in statements ---
    {
        IfStatement ifStmt;
        BooleanLiteral* cond = new BooleanLiteral("b1", false);
        ifStmt.setChild("condition", cond);
        Return* thenRet = new Return();
        IntegerLiteral* thenVal = new IntegerLiteral("i1", 1);
        thenRet->setChild("value", thenVal);
        ifStmt.addChild("thenBranch", thenRet);

        CppGenerator gen;
        std::string output = gen.generate(&ifStmt);

        // Python if uses colon and indentation; C++ uses braces
        assert(notContains(output, "elif") && "C++ should not contain 'elif'");
        assert(notContains(output, "pass") && "C++ should not contain 'pass'");

        std::cout << "Test 6 PASS: Statements do not contain Python syntax" << std::endl;
        ++passed;

        delete thenVal;
        delete thenRet;
        delete cond;
    }

    // --- Summary ---
    std::cout << "\n=== Step 40 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
