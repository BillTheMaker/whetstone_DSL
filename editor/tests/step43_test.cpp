// Step 43 TDD Test: Memory strategy code generation
//
// Tests the refactoring from DerefStrategy to canonical annotation types
// and their effect on C++ code generation:
// 1. DeallocateAnnotation("Explicit") → raw pointers with new/delete
// 2. LifetimeAnnotation("RAII") → std::unique_ptr<T>, RAII destructors
// 3. ReclaimAnnotation("Tracing") → std::shared_ptr<T> for GC-like ref counting
// 4. OwnerAnnotation("Single") → std::unique_ptr<T> with strict ownership
// 5. OwnerAnnotation("Shared_ARC") → std::shared_ptr<T> with ARC semantics
// 6. New annotation classes exist and inherit from Annotation
// 7. DerefStrategy still works as deprecated wrapper
//
// Will fail to compile until canonical annotation classes are created in Annotation.h.

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

    // --- Test 1: DeallocateAnnotation exists and inherits from Annotation ---
    {
        DeallocateAnnotation anno;
        anno.strategy = "Explicit";
        Annotation* base = &anno;
        (void)base;
        assert(anno.strategy == "Explicit" && "Should store strategy");

        std::cout << "Test 1 PASS: DeallocateAnnotation exists with strategy field" << std::endl;
        ++passed;
    }

    // --- Test 2: LifetimeAnnotation exists ---
    {
        LifetimeAnnotation anno;
        anno.strategy = "RAII";
        Annotation* base = &anno;
        (void)base;
        assert(anno.strategy == "RAII" && "Should store strategy");

        std::cout << "Test 2 PASS: LifetimeAnnotation exists with strategy field" << std::endl;
        ++passed;
    }

    // --- Test 3: ReclaimAnnotation exists ---
    {
        ReclaimAnnotation anno;
        anno.strategy = "Tracing";
        Annotation* base = &anno;
        (void)base;
        assert(anno.strategy == "Tracing" && "Should store strategy");

        std::cout << "Test 3 PASS: ReclaimAnnotation exists with strategy field" << std::endl;
        ++passed;
    }

    // --- Test 4: OwnerAnnotation exists ---
    {
        OwnerAnnotation anno;
        anno.strategy = "Single";
        Annotation* base = &anno;
        (void)base;
        assert(anno.strategy == "Single" && "Should store strategy");

        std::cout << "Test 4 PASS: OwnerAnnotation exists with strategy field" << std::endl;
        ++passed;
    }

    // --- Test 5: AllocateAnnotation exists ---
    {
        AllocateAnnotation anno;
        anno.strategy = "Static";
        Annotation* base = &anno;
        (void)base;
        assert(anno.strategy == "Static" && "Should store strategy");

        std::cout << "Test 5 PASS: AllocateAnnotation exists with strategy field" << std::endl;
        ++passed;
    }

    // --- Test 6: @Deallocate(Explicit) → raw pointer in C++ ---
    {
        // Build a function with DeallocateAnnotation
        Function fn("f1", "allocateWidget");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn.setChild("returnType", retType);

        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->strategy = "Explicit";
        fn.addChild("annotations", anno);

        // Add a variable declaration in body
        Variable* var = new Variable("v1", "widget");
        CustomType* widgetType = new CustomType("t2", "Widget");
        var->setChild("type", widgetType);
        fn.addChild("body", var);

        CppGenerator gen;
        std::string output = gen.generate(&fn);

        // With @Deallocate(Explicit), should use raw pointers
        assert(contains(output, "Widget") && "Should contain the type name");
        assert(contains(output, "allocateWidget") && "Should contain function name");
        // Should produce raw pointer syntax (Widget* or new Widget)
        assert((contains(output, "*") || contains(output, "new")) &&
               "@Deallocate(Explicit) should produce raw pointer or new operator");

        std::cout << "Test 6 PASS: @Deallocate(Explicit) produces raw pointer code" << std::endl;
        ++passed;

        delete widgetType;
        delete var;
        delete anno;
        delete retType;
    }

    // --- Test 7: @Lifetime(RAII) → unique_ptr in C++ ---
    {
        Function fn("f1", "createWidget");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn.setChild("returnType", retType);

        LifetimeAnnotation* anno = new LifetimeAnnotation();
        anno->strategy = "RAII";
        fn.addChild("annotations", anno);

        Variable* var = new Variable("v1", "widget");
        CustomType* widgetType = new CustomType("t2", "Widget");
        var->setChild("type", widgetType);
        fn.addChild("body", var);

        CppGenerator gen;
        std::string output = gen.generate(&fn);

        assert(contains(output, "unique_ptr") && "@Lifetime(RAII) should produce unique_ptr");

        std::cout << "Test 7 PASS: @Lifetime(RAII) produces unique_ptr code" << std::endl;
        ++passed;

        delete widgetType;
        delete var;
        delete anno;
        delete retType;
    }

    // --- Test 8: @Reclaim(Tracing) → shared_ptr in C++ ---
    {
        Function fn("f1", "getWidget");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn.setChild("returnType", retType);

        ReclaimAnnotation* anno = new ReclaimAnnotation();
        anno->strategy = "Tracing";
        fn.addChild("annotations", anno);

        Variable* var = new Variable("v1", "widget");
        CustomType* widgetType = new CustomType("t2", "Widget");
        var->setChild("type", widgetType);
        fn.addChild("body", var);

        CppGenerator gen;
        std::string output = gen.generate(&fn);

        assert(contains(output, "shared_ptr") && "@Reclaim(Tracing) should produce shared_ptr");

        std::cout << "Test 8 PASS: @Reclaim(Tracing) produces shared_ptr code" << std::endl;
        ++passed;

        delete widgetType;
        delete var;
        delete anno;
        delete retType;
    }

    // --- Test 9: @Owner(Shared_ARC) → shared_ptr in C++ ---
    {
        Function fn("f1", "shareWidget");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn.setChild("returnType", retType);

        OwnerAnnotation* anno = new OwnerAnnotation();
        anno->strategy = "Shared_ARC";
        fn.addChild("annotations", anno);

        Variable* var = new Variable("v1", "widget");
        CustomType* widgetType = new CustomType("t2", "Widget");
        var->setChild("type", widgetType);
        fn.addChild("body", var);

        CppGenerator gen;
        std::string output = gen.generate(&fn);

        assert(contains(output, "shared_ptr") && "@Owner(Shared_ARC) should produce shared_ptr");

        std::cout << "Test 9 PASS: @Owner(Shared_ARC) produces shared_ptr code" << std::endl;
        ++passed;

        delete widgetType;
        delete var;
        delete anno;
        delete retType;
    }

    // --- Test 10: Old DerefStrategy still works (backward compat) ---
    {
        DerefStrategy deref("d1", "imperative");
        Annotation* base = &deref;
        (void)base;
        assert(deref.strategy == "imperative" && "Old DerefStrategy should still work");

        std::cout << "Test 10 PASS: DerefStrategy still functional (backward compat)" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 43 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
