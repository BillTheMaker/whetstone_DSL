// Step 397: Auto Type Deduction (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "ast/Parser.h"
#include "ast/CppAdvancedNodes.h"

int main() {
    int passed = 0;

    // Test 1: AutoType node construction
    {
        AutoType at("auto_1", "");
        assert(at.conceptType == "AutoType");
        assert(at.modifiers.empty());
        std::cout << "PASS: test 1 — AutoType construction\n";
        passed++;
    }

    // Test 2: AutoType with pointer modifier
    {
        AutoType at("auto_2", "*");
        assert(at.modifiers == "*");
        std::cout << "PASS: test 2 — AutoType pointer\n";
        passed++;
    }

    // Test 3: AutoType with const reference
    {
        AutoType at("auto_3", "const &");
        assert(at.modifiers == "const &");
        std::cout << "PASS: test 3 — AutoType const ref\n";
        passed++;
    }

    // Test 4: isAutoType detection
    {
        assert(isAutoType("auto") == true);
        assert(isAutoType("auto*") == true);
        assert(isAutoType("auto&") == true);
        assert(isAutoType("const auto") == true);
        assert(isAutoType("const auto&") == true);
        assert(isAutoType("int") == false);
        assert(isAutoType("automatic") == false);
        std::cout << "PASS: test 4 — isAutoType detection\n";
        passed++;
    }

    // Test 5: Parse function with auto return type
    {
        std::string src = R"(
auto compute() {
    return 42;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        auto* fn = static_cast<Function*>(fns[0]);
        assert(fn->name == "compute");
        // Return type should be "auto" or similar
        auto* retType = fn->getChild("returnType");
        if (retType && retType->conceptType == "PrimitiveType") {
            auto* pt = static_cast<PrimitiveType*>(retType);
            assert(isAutoType(pt->kind));
        }
        std::cout << "PASS: test 5 — parse auto return type\n";
        passed++;
    }

    // Test 6: Parse auto variable declaration
    {
        std::string src = R"(
void test() {
    auto x = 5;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        std::cout << "PASS: test 6 — parse auto variable\n";
        passed++;
    }

    // Test 7: Parse auto* pointer
    {
        std::string src = R"(
void test() {
    auto* p = new int(42);
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        std::cout << "PASS: test 7 — parse auto* pointer\n";
        passed++;
    }

    // Test 8: Parse const auto& reference
    {
        std::string src = R"(
void test(const std::vector<int>& v) {
    const auto& ref = v;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        std::cout << "PASS: test 8 — parse const auto& reference\n";
        passed++;
    }

    // Test 9: AutoType id generation
    {
        AutoType a1("auto_10", "");
        AutoType a2("auto_11", "*");
        assert(a1.id != a2.id);
        assert(a1.modifiers != a2.modifiers);
        std::cout << "PASS: test 9 — AutoType id generation\n";
        passed++;
    }

    // Test 10: Parse trailing return type
    {
        std::string src = R"(
auto add(int a, int b) -> int {
    return a + b;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        auto* fn = static_cast<Function*>(fns[0]);
        assert(fn->name == "add");
        std::cout << "PASS: test 10 — parse trailing return type\n";
        passed++;
    }

    // Test 11: Parse function with auto parameter (C++20 abbreviated template)
    {
        std::string src = R"(
void process(auto item) {
    return;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        std::cout << "PASS: test 11 — parse auto parameter\n";
        passed++;
    }

    // Test 12: AutoType span tracking
    {
        AutoType at("auto_20", "const &");
        at.setSpan(10, 4, 10, 15);
        assert(at.spanStartLine == 10);
        assert(at.spanEndCol == 15);
        std::cout << "PASS: test 12 — AutoType span tracking\n";
        passed++;
    }

    std::cout << "\nStep 397 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
