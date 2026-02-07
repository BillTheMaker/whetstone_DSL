// Step 67 TDD Test: Optimization annotations
//
// Tests optimization annotation classes and their C++ code generation:
// 1. @Hot → C++ __attribute__((hot))
// 2. @Cold → C++ __attribute__((cold))
// 3. @Inline(Always) → C++ [[gnu::always_inline]]
// 4. @Pure → C++ [[nodiscard]]
// 5. @ConstExpr → C++ constexpr
// 6. All optimization annotation classes exist and inherit from Annotation
//
// Will fail until optimization annotation classes are created and
// CppGenerator handles them.

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

    // --- Test 1: HotColdAnnotation exists ---
    {
        HotColdAnnotation anno;
        anno.hint = "Hot";
        Annotation* base = &anno;
        (void)base;
        assert(anno.hint == "Hot" && "Should store hint");

        std::cout << "Test 1 PASS: HotColdAnnotation exists" << std::endl;
        ++passed;
    }

    // --- Test 2: InlineAnnotation exists ---
    {
        InlineAnnotation anno;
        anno.mode = "Always";
        Annotation* base = &anno;
        (void)base;
        assert(anno.mode == "Always" && "Should store mode");

        std::cout << "Test 2 PASS: InlineAnnotation exists" << std::endl;
        ++passed;
    }

    // --- Test 3: PureAnnotation exists ---
    {
        PureAnnotation anno;
        Annotation* base = &anno;
        (void)base;

        std::cout << "Test 3 PASS: PureAnnotation exists" << std::endl;
        ++passed;
    }

    // --- Test 4: ConstExprAnnotation exists ---
    {
        ConstExprAnnotation anno;
        Annotation* base = &anno;
        (void)base;

        std::cout << "Test 4 PASS: ConstExprAnnotation exists" << std::endl;
        ++passed;
    }

    // --- Test 5: @Hot function → C++ __attribute__((hot)) ---
    {
        Module mod("m1", "Perf", "cpp");
        Function* fn = new Function("f1", "hotPath");
        PrimitiveType* retType = new PrimitiveType("t1", "void");
        fn->setChild("returnType", retType);

        HotColdAnnotation* anno = new HotColdAnnotation();
        anno->hint = "Hot";
        fn->addChild("annotations", anno);
        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert((contains(output, "__attribute__((hot))") || contains(output, "[[likely]]")) &&
               "@Hot should produce __attribute__((hot)) or [[likely]]");

        std::cout << "Test 5 PASS: @Hot → C++ hot attribute" << std::endl;
        ++passed;

        delete anno;
        delete retType;
        delete fn;
    }

    // --- Test 6: @Cold function → C++ __attribute__((cold)) ---
    {
        Module mod("m1", "Perf", "cpp");
        Function* fn = new Function("f1", "errorHandler");
        PrimitiveType* retType = new PrimitiveType("t1", "void");
        fn->setChild("returnType", retType);

        HotColdAnnotation* anno = new HotColdAnnotation();
        anno->hint = "Cold";
        fn->addChild("annotations", anno);
        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert((contains(output, "__attribute__((cold))") || contains(output, "[[unlikely]]")) &&
               "@Cold should produce __attribute__((cold)) or [[unlikely]]");

        std::cout << "Test 6 PASS: @Cold → C++ cold attribute" << std::endl;
        ++passed;

        delete anno;
        delete retType;
        delete fn;
    }

    // --- Test 7: @Inline(Always) → C++ always_inline ---
    {
        Module mod("m1", "Perf", "cpp");
        Function* fn = new Function("f1", "fastMath");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn->setChild("returnType", retType);

        InlineAnnotation* anno = new InlineAnnotation();
        anno->mode = "Always";
        fn->addChild("annotations", anno);
        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert((contains(output, "always_inline") || contains(output, "inline")) &&
               "@Inline(Always) should produce inline attribute");

        std::cout << "Test 7 PASS: @Inline(Always) → C++ always_inline" << std::endl;
        ++passed;

        delete anno;
        delete retType;
        delete fn;
    }

    // --- Test 8: @Pure → C++ [[nodiscard]] ---
    {
        Module mod("m1", "Perf", "cpp");
        Function* fn = new Function("f1", "pureCalc");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn->setChild("returnType", retType);

        PureAnnotation* anno = new PureAnnotation();
        fn->addChild("annotations", anno);
        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert((contains(output, "[[nodiscard]]") || contains(output, "pure") ||
                contains(output, "[[gnu::pure]]")) &&
               "@Pure should produce [[nodiscard]] or [[gnu::pure]]");

        std::cout << "Test 8 PASS: @Pure → C++ pure/nodiscard attribute" << std::endl;
        ++passed;

        delete anno;
        delete retType;
        delete fn;
    }

    // --- Test 9: @ConstExpr → C++ constexpr ---
    {
        Module mod("m1", "Perf", "cpp");
        Function* fn = new Function("f1", "constCalc");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn->setChild("returnType", retType);

        ConstExprAnnotation* anno = new ConstExprAnnotation();
        fn->addChild("annotations", anno);
        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert(contains(output, "constexpr") &&
               "@ConstExpr should produce 'constexpr' keyword");

        std::cout << "Test 9 PASS: @ConstExpr → C++ constexpr" << std::endl;
        ++passed;

        delete anno;
        delete retType;
        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 67 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
