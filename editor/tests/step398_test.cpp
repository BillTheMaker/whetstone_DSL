// Step 398: Cast Expressions (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "ast/Parser.h"
#include "ast/CppAdvancedNodes.h"

int main() {
    int passed = 0;

    // Test 1: CastExpression node construction
    {
        CastExpression ce("cast_1", "static_cast", "int");
        assert(ce.conceptType == "CastExpression");
        assert(ce.castKind == "static_cast");
        assert(ce.targetType == "int");
        std::cout << "PASS: test 1 — CastExpression construction\n";
        passed++;
    }

    // Test 2: Detect static_cast
    {
        assert(detectCastKind("static_cast<int>(x)") == "static_cast");
        std::cout << "PASS: test 2 — detect static_cast\n";
        passed++;
    }

    // Test 3: Detect dynamic_cast
    {
        assert(detectCastKind("dynamic_cast<Derived*>(base)") == "dynamic_cast");
        std::cout << "PASS: test 3 — detect dynamic_cast\n";
        passed++;
    }

    // Test 4: Detect reinterpret_cast
    {
        assert(detectCastKind("reinterpret_cast<char*>(ptr)") == "reinterpret_cast");
        std::cout << "PASS: test 4 — detect reinterpret_cast\n";
        passed++;
    }

    // Test 5: Detect const_cast
    {
        assert(detectCastKind("const_cast<int*>(cptr)") == "const_cast");
        std::cout << "PASS: test 5 — detect const_cast\n";
        passed++;
    }

    // Test 6: Risk level for casts
    {
        assert(castRiskLevel("static_cast") == "low");
        assert(castRiskLevel("dynamic_cast") == "medium");
        assert(castRiskLevel("const_cast") == "medium");
        assert(castRiskLevel("reinterpret_cast") == "high");
        std::cout << "PASS: test 6 — cast risk levels\n";
        passed++;
    }

    // Test 7: Parse static_cast in function
    {
        std::string src = R"(
void convert() {
    double d = 3.14;
    int i = static_cast<int>(d);
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        auto* fn = static_cast<Function*>(fns[0]);
        assert(fn->name == "convert");
        std::cout << "PASS: test 7 — parse static_cast in function\n";
        passed++;
    }

    // Test 8: Parse dynamic_cast in function
    {
        std::string src = R"(
class Base { virtual ~Base() {} };
class Derived : public Base {};
void check(Base* b) {
    auto* d = dynamic_cast<Derived*>(b);
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        std::cout << "PASS: test 8 — parse dynamic_cast in function\n";
        passed++;
    }

    // Test 9: CastExpression with operand child
    {
        CastExpression ce("cast_2", "static_cast", "const Foo*");
        auto* operand = new VariableReference("ref_1", "bar");
        ce.setChild("operand", operand);

        auto* op = ce.getChild("operand");
        assert(op != nullptr);
        assert(op->conceptType == "VariableReference");
        std::cout << "PASS: test 9 — CastExpression with operand child\n";
        passed++;
    }

    // Test 10: Multiple casts in one function (no crash)
    {
        std::string src = R"(
void multicast() {
    int x = 42;
    double d = static_cast<double>(x);
    const int* cp = &x;
    int* p = const_cast<int*>(cp);
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        std::cout << "PASS: test 10 — multiple casts in one function\n";
        passed++;
    }

    // Test 11: CastExpression span tracking
    {
        CastExpression ce("cast_3", "dynamic_cast", "Widget*");
        ce.setSpan(15, 8, 15, 40);
        assert(ce.spanStartLine == 15);
        assert(ce.spanEndCol == 40);
        std::cout << "PASS: test 11 — CastExpression span tracking\n";
        passed++;
    }

    // Test 12: Risk annotation mapping for all cast kinds
    {
        std::vector<std::pair<std::string, std::string>> expected = {
            {"static_cast", "low"},
            {"dynamic_cast", "medium"},
            {"const_cast", "medium"},
            {"reinterpret_cast", "high"}
        };
        for (const auto& [kind, risk] : expected) {
            CastExpression ce("cast_x", kind, "void*");
            assert(castRiskLevel(ce.castKind) == risk);
        }
        std::cout << "PASS: test 12 — risk annotation mapping\n";
        passed++;
    }

    std::cout << "\nStep 398 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
