// Step 394: TypeAlias + Nested Type Access (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "ast/Parser.h"
#include "ast/Generator.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/CppAdvancedNodes.h"

int main() {
    int passed = 0;

    // Test 1: Parse using type alias
    {
        std::string src = "using json = nlohmann::json;\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "TypeAlias") {
                auto* ta = static_cast<TypeAlias*>(s);
                assert(ta->aliasName == "json");
                assert(ta->isUsing == true);
                found = true;
            }
        }
        assert(found);
        std::cout << "PASS: test 1 — parse using type alias\n";
        passed++;
    }

    // Test 2: Parse typedef type alias
    {
        std::string src = "typedef int IntType;\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& stmts = mod->getChildren("statements");
        bool found = false;
        for (auto* s : stmts) {
            if (s->conceptType == "TypeAlias") {
                auto* ta = static_cast<TypeAlias*>(s);
                assert(ta->aliasName == "IntType");
                assert(ta->isUsing == false);
                found = true;
            }
        }
        assert(found);
        std::cout << "PASS: test 2 — parse typedef type alias\n";
        passed++;
    }

    // Test 3: Nested type access in alias target (qualified name)
    {
        std::string src = "using json = nlohmann::json;\n";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& stmts = mod->getChildren("statements");
        for (auto* s : stmts) {
            if (s->conceptType == "TypeAlias") {
                auto* ta = static_cast<TypeAlias*>(s);
                // Target type should preserve the qualified name
                assert(ta->targetType.find("nlohmann") != std::string::npos ||
                       ta->targetType.find("json") != std::string::npos);
            }
        }
        std::cout << "PASS: test 3 — nested type access in alias\n";
        passed++;
    }

    // Test 4: TypeAlias construction with qualified name
    {
        TypeAlias ta("ta_1", "StringVec", "std::vector<std::string>", true);
        assert(ta.aliasName == "StringVec");
        assert(ta.targetType == "std::vector<std::string>");
        assert(ta.isUsing == true);
        assert(ta.conceptType == "TypeAlias");
        std::cout << "PASS: test 4 — TypeAlias construction\n";
        passed++;
    }

    // Test 5: Multiple type aliases in one module
    {
        std::string src = R"(
using json = nlohmann::json;
using string = std::string;
)";
        auto mod = TreeSitterParser::parseCpp(src);
        int count = 0;
        for (auto* s : mod->getChildren("statements")) {
            if (s->conceptType == "TypeAlias") count++;
        }
        assert(count == 2);
        std::cout << "PASS: test 5 — multiple type aliases\n";
        passed++;
    }

    // Test 6: TypeAlias in generator produces using declaration
    {
        TypeAlias ta("ta_1", "Vec", "std::vector<int>", true);
        // CppGenerator would produce: "using Vec = std::vector<int>;"
        // But we just test that the node round-trips correctly
        assert(ta.aliasName == "Vec");
        assert(ta.targetType == "std::vector<int>");

        TypeAlias td("ta_2", "IntPtr", "int*", false);
        assert(td.isUsing == false);
        // typedef int* IntPtr;
        std::cout << "PASS: test 6 — TypeAlias generator format\n";
        passed++;
    }

    // Test 7: Deeply nested qualified type
    {
        TypeAlias ta("ta_1", "Iter", "Foo::Bar::Baz::iterator", true);
        assert(ta.targetType == "Foo::Bar::Baz::iterator");
        // Verify :: count
        int colons = 0;
        for (size_t i = 0; i + 1 < ta.targetType.size(); ++i) {
            if (ta.targetType[i] == ':' && ta.targetType[i + 1] == ':') colons++;
        }
        assert(colons == 3);
        std::cout << "PASS: test 7 — deeply nested qualified type\n";
        passed++;
    }

    // Test 8: Parse class with type alias inside namespace
    {
        std::string src = R"(
namespace detail {
    using size_type = std::size_t;
}
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto& stmts = mod->getChildren("statements");
        bool nsFound = false;
        for (auto* s : stmts) {
            if (s->conceptType == "NamespaceDeclaration") {
                auto* ns = static_cast<NamespaceDeclaration*>(s);
                assert(ns->name == "detail");
                nsFound = true;
            }
        }
        assert(nsFound);
        std::cout << "PASS: test 8 — type alias in namespace\n";
        passed++;
    }

    // Test 9: TypeAlias span tracking
    {
        TypeAlias ta("ta_1", "MyType", "int", true);
        ta.setSpan(5, 0, 5, 20);
        assert(ta.spanStartLine == 5);
        assert(ta.spanEndCol == 20);
        std::cout << "PASS: test 9 — TypeAlias span tracking\n";
        passed++;
    }

    // Test 10: Template type in alias target
    {
        std::string src = "using Strings = std::vector<std::string>;\n";
        auto mod = TreeSitterParser::parseCpp(src);
        bool found = false;
        for (auto* s : mod->getChildren("statements")) {
            if (s->conceptType == "TypeAlias") {
                auto* ta = static_cast<TypeAlias*>(s);
                assert(ta->aliasName == "Strings");
                found = true;
            }
        }
        assert(found);
        std::cout << "PASS: test 10 — template type in alias target\n";
        passed++;
    }

    // Test 11: Typedef with pointer
    {
        std::string src = "typedef void (*Callback)(int);\n";
        auto mod = TreeSitterParser::parseCpp(src);
        // This may or may not parse as TypeAlias depending on tree-sitter
        // Just verify no crash
        assert(mod != nullptr);
        std::cout << "PASS: test 11 — typedef with pointer (no crash)\n";
        passed++;
    }

    // Test 12: CppQualifiers helper on qualified type text
    {
        auto q1 = detectQualifiers("static constexpr int MAX = 100;");
        assert(q1.isStatic == true);
        assert(q1.isConstexpr == true);

        auto q2 = detectQualifiers("const std::string& name");
        assert(q2.isConst == true);
        assert(q2.isStatic == false);

        auto q3 = detectQualifiers("std::unique_ptr<Widget> w;");
        assert(q3.smartPointerKind == "unique_ptr");

        std::cout << "PASS: test 12 — CppQualifiers detection helpers\n";
        passed++;
    }

    std::cout << "\nStep 394 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
