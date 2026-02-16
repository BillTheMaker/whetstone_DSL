// Step 396: Smart Pointer Patterns (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "ast/Parser.h"
#include "ast/CppAdvancedNodes.h"

int main() {
    int passed = 0;

    // Test 1: Detect unique_ptr pattern
    {
        std::string text = "std::unique_ptr<Widget> widget;";
        assert(detectSmartPointerKind(text) == "unique_ptr");
        std::cout << "PASS: test 1 — detect unique_ptr\n";
        passed++;
    }

    // Test 2: Detect shared_ptr pattern
    {
        std::string text = "std::shared_ptr<Config> config;";
        assert(detectSmartPointerKind(text) == "shared_ptr");
        std::cout << "PASS: test 2 — detect shared_ptr\n";
        passed++;
    }

    // Test 3: Detect weak_ptr pattern
    {
        std::string text = "std::weak_ptr<Observer> obs;";
        assert(detectSmartPointerKind(text) == "weak_ptr");
        std::cout << "PASS: test 3 — detect weak_ptr\n";
        passed++;
    }

    // Test 4: Extract inner type from smart pointer
    {
        assert(extractSmartPointerInner("std::unique_ptr<Widget>") == "Widget");
        assert(extractSmartPointerInner("shared_ptr<const Foo>") == "const Foo");
        assert(extractSmartPointerInner("std::shared_ptr<std::vector<int>>") == "std::vector<int>");
        std::cout << "PASS: test 4 — extract inner type\n";
        passed++;
    }

    // Test 5: Smart pointer ownership mapping
    {
        assert(smartPointerOwnership("unique_ptr") == "Unique");
        assert(smartPointerOwnership("shared_ptr") == "Shared_ARC");
        assert(smartPointerOwnership("weak_ptr") == "Weak");
        assert(smartPointerOwnership("") == "");
        std::cout << "PASS: test 5 — ownership mapping\n";
        passed++;
    }

    // Test 6: Parse function with unique_ptr parameter
    {
        std::string src = R"(
void process(std::unique_ptr<Widget> w) {
    return;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        auto* fn = static_cast<Function*>(fns[0]);
        assert(fn->name == "process");
        std::cout << "PASS: test 6 — parse function with unique_ptr param\n";
        passed++;
    }

    // Test 7: Parse function with shared_ptr return
    {
        std::string src = R"(
std::shared_ptr<Config> createConfig() {
    return nullptr;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        auto* fn = static_cast<Function*>(fns[0]);
        assert(fn->name == "createConfig");
        // Check return type contains shared_ptr
        auto* retType = fn->getChild("returnType");
        if (retType && retType->conceptType == "PrimitiveType") {
            auto* pt = static_cast<PrimitiveType*>(retType);
            assert(detectSmartPointerKind(pt->kind) == "shared_ptr");
        }
        std::cout << "PASS: test 7 — parse function with shared_ptr return\n";
        passed++;
    }

    // Test 8: Detect smart pointer in class field declaration text
    {
        auto q = detectQualifiers("std::unique_ptr<Widget> widget_;");
        assert(q.smartPointerKind == "unique_ptr");

        auto q2 = detectQualifiers("std::shared_ptr<Logger> logger_;");
        assert(q2.smartPointerKind == "shared_ptr");
        std::cout << "PASS: test 8 — smart pointer in field text\n";
        passed++;
    }

    // Test 9: No false positive on non-smart-pointer
    {
        assert(detectSmartPointerKind("int x;") == "");
        assert(detectSmartPointerKind("std::vector<int> v;") == "");
        assert(detectSmartPointerKind("auto ptr = new Widget();") == "");
        std::cout << "PASS: test 9 — no false positive\n";
        passed++;
    }

    // Test 10: Parse make_unique call
    {
        std::string src = R"(
void init() {
    auto w = std::make_unique<Widget>();
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        std::cout << "PASS: test 10 — parse make_unique call\n";
        passed++;
    }

    // Test 11: Smart pointer cross-language mapping concept
    {
        // unique_ptr → Rust Box<T>, shared_ptr → Rust Arc<T>
        std::string kind1 = "unique_ptr";
        std::string ownership1 = smartPointerOwnership(kind1);
        assert(ownership1 == "Unique"); // Would map to Box<T> in Rust generator

        std::string kind2 = "shared_ptr";
        std::string ownership2 = smartPointerOwnership(kind2);
        assert(ownership2 == "Shared_ARC"); // Would map to Arc<T> in Rust generator
        std::cout << "PASS: test 11 — cross-language mapping concept\n";
        passed++;
    }

    // Test 12: Parse new/delete expressions (FunctionCall nodes)
    {
        std::string src = R"(
void cleanup() {
    auto* p = new Widget();
    delete p;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        auto& fns = mod->getChildren("functions");
        assert(!fns.empty());
        std::cout << "PASS: test 12 — parse new/delete expressions\n";
        passed++;
    }

    std::cout << "\nStep 396 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
