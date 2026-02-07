// Step 46 TDD Test: Tree-sitter C++ integration
//
// Tests that TreeSitterParser::parseCpp() actually parses real C++ source:
// 1. Simple function → Function node with correct name and return type
// 2. Parameters with types → Parameter nodes with PrimitiveType children
// 3. Variable declarations → Variable nodes
// 4. Detect unique_ptr → @Lifetime(RAII) annotation
// 5. Detect shared_ptr → @Owner(Shared_ARC) annotation
// 6. Detect raw new/delete → @Deallocate(Explicit) annotation
// 7. Module targetLanguage is "cpp"
//
// Will fail until TreeSitterParser::parseCpp() is replaced with real
// tree-sitter-cpp integration.

#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "ast/Parser.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Parse simple C++ function ---
    {
        std::string source = "int f(int x) { return x + 1; }";
        auto module = TreeSitterParser::parseCpp(source);

        assert(module != nullptr && "parseCpp should return a non-null module");
        assert(module->targetLanguage == "cpp" && "Module target language should be 'cpp'");

        std::cout << "Test 1 PASS: parseCpp returns non-null module with cpp target" << std::endl;
        ++passed;
    }

    // --- Test 2: Parsed C++ function has correct name ---
    {
        std::string source = "int f(int x) { return x + 1; }";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty() && "Module should contain at least one function");

        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "f" && "Function name should be 'f'");

        std::cout << "Test 2 PASS: Parsed C++ function has name 'f'" << std::endl;
        ++passed;
    }

    // --- Test 3: C++ function has typed parameter ---
    {
        std::string source = "int f(int x) { return x + 1; }";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

        auto params = fn->getChildren("parameters");
        assert(!params.empty() && "Function should have parameters");

        auto* param = static_cast<Parameter*>(params[0]);
        assert(param->name == "x" && "Parameter name should be 'x'");

        auto* type = param->getChild("type");
        assert(type != nullptr && "Parameter should have a type");
        assert(type->conceptType == "PrimitiveType" && "Type should be PrimitiveType");

        auto* primType = static_cast<PrimitiveType*>(type);
        assert(primType->kind == "int" && "Parameter type should be 'int'");

        std::cout << "Test 3 PASS: C++ parameter has name 'x' with type 'int'" << std::endl;
        ++passed;
    }

    // --- Test 4: Function has return type ---
    {
        std::string source = "int f(int x) { return x + 1; }";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);

        auto* retType = fn->getChild("returnType");
        assert(retType != nullptr && "Function should have a return type");
        assert(retType->conceptType == "PrimitiveType" && "Return type should be PrimitiveType");

        auto* primType = static_cast<PrimitiveType*>(retType);
        assert(primType->kind == "int" && "Return type should be 'int'");

        std::cout << "Test 4 PASS: C++ function has return type 'int'" << std::endl;
        ++passed;
    }

    // --- Test 5: Detect unique_ptr → @Lifetime(RAII) ---
    {
        std::string source =
            "void createWidget() {\n"
            "    std::unique_ptr<Widget> w = std::make_unique<Widget>();\n"
            "}\n";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty() && "Should parse the function");
        auto* fn = static_cast<Function*>(functions[0]);

        // Look for @Lifetime(RAII) annotation inferred from unique_ptr usage
        auto annotations = fn->getChildren("annotations");
        bool hasLifetimeRAII = false;
        for (auto* anno : annotations) {
            if (anno->conceptType == "LifetimeAnnotation") {
                auto* lifetime = static_cast<LifetimeAnnotation*>(anno);
                if (lifetime->strategy == "RAII") {
                    hasLifetimeRAII = true;
                }
            }
        }
        assert(hasLifetimeRAII &&
               "unique_ptr usage should be annotated with @Lifetime(RAII)");

        std::cout << "Test 5 PASS: unique_ptr detected → @Lifetime(RAII)" << std::endl;
        ++passed;
    }

    // --- Test 6: Detect shared_ptr → @Owner(Shared_ARC) ---
    {
        std::string source =
            "void shareWidget() {\n"
            "    std::shared_ptr<Widget> w = std::make_shared<Widget>();\n"
            "}\n";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty() && "Should parse the function");
        auto* fn = static_cast<Function*>(functions[0]);

        auto annotations = fn->getChildren("annotations");
        bool hasSharedARC = false;
        for (auto* anno : annotations) {
            if (anno->conceptType == "OwnerAnnotation") {
                auto* owner = static_cast<OwnerAnnotation*>(anno);
                if (owner->strategy == "Shared_ARC") {
                    hasSharedARC = true;
                }
            }
        }
        assert(hasSharedARC &&
               "shared_ptr usage should be annotated with @Owner(Shared_ARC)");

        std::cout << "Test 6 PASS: shared_ptr detected → @Owner(Shared_ARC)" << std::endl;
        ++passed;
    }

    // --- Test 7: Detect raw new/delete → @Deallocate(Explicit) ---
    {
        std::string source =
            "void rawAlloc() {\n"
            "    Widget* w = new Widget();\n"
            "    delete w;\n"
            "}\n";
        auto module = TreeSitterParser::parseCpp(source);

        auto functions = module->getChildren("functions");
        assert(!functions.empty() && "Should parse the function");
        auto* fn = static_cast<Function*>(functions[0]);

        auto annotations = fn->getChildren("annotations");
        bool hasDeallocExplicit = false;
        for (auto* anno : annotations) {
            if (anno->conceptType == "DeallocateAnnotation") {
                auto* dealloc = static_cast<DeallocateAnnotation*>(anno);
                if (dealloc->strategy == "Explicit") {
                    hasDeallocExplicit = true;
                }
            }
        }
        assert(hasDeallocExplicit &&
               "raw new/delete should be annotated with @Deallocate(Explicit)");

        std::cout << "Test 7 PASS: raw new/delete detected → @Deallocate(Explicit)" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 46 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
