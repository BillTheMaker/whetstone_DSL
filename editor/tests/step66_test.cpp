// Step 66 TDD Test: Cross-language memory projection
//
// Tests the "Lossless Projection Logic" from Memory strategy.md Section 3:
// 1. Python → C++: @Reclaim(Tracing) → inject ref-counting shim (shared_ptr)
// 2. C++ → Python: @Deallocate(Explicit) → wrap in try-with-resources pattern
// 3. Rust-like → Python: @Owner(Single) → preserve in metadata, don't enforce
// 4. Round-trip: annotation survives language round-trip
// 5. @Lifetime(RAII) → inject destructor calls at scope end
//
// Will fail until cross-language projection is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Annotation.h"
#include "ast/Generator.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// Forward declaration — CrossLanguageProjector
class CrossLanguageProjector {
public:
    // Project an AST from source language to target language
    // Returns a new AST with annotations adapted for the target
    std::unique_ptr<Module> project(const Module* source, const std::string& targetLanguage) const;

    // Check if all annotations survived the projection
    bool annotationsPreserved(const Module* original, const Module* projected) const;
};

int main() {
    int passed = 0;
    int failed = 0;

    CrossLanguageProjector projector;

    // --- Test 1: Python @Reclaim(Tracing) → C++ shared_ptr ---
    {
        Module pyMod("m1", "PyModule", "python");
        Function* fn = new Function("f1", "process");

        ReclaimAnnotation* anno = new ReclaimAnnotation();
        anno->id = "a1";
        anno->strategy = "Tracing";
        fn->addChild("annotations", anno);

        Variable* var = new Variable("v1", "data");
        fn->addChild("body", var);
        pyMod.addChild("functions", fn);

        auto cppMod = projector.project(&pyMod, "cpp");
        assert(cppMod != nullptr && "Projection should produce a module");
        assert(cppMod->targetLanguage == "cpp" && "Target should be cpp");

        // Generate C++ from the projected module
        CppGenerator gen;
        std::string output = gen.generate(cppMod.get());

        // @Reclaim(Tracing) in C++ → should use shared_ptr for GC-like semantics
        assert(contains(output, "shared_ptr") &&
               "Python @Reclaim(Tracing) → C++ should use shared_ptr");

        std::cout << "Test 1 PASS: Python @Reclaim(Tracing) → C++ shared_ptr" << std::endl;
        ++passed;

        delete var;
        delete anno;
        delete fn;
    }

    // --- Test 2: C++ @Deallocate(Explicit) → Python wrapper ---
    {
        Module cppMod("m1", "CppModule", "cpp");
        Function* fn = new Function("f1", "allocate");

        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Explicit";
        fn->addChild("annotations", anno);

        Variable* var = new Variable("v1", "ptr");
        fn->addChild("body", var);
        cppMod.addChild("functions", fn);

        auto pyMod = projector.project(&cppMod, "python");
        assert(pyMod != nullptr && "Projection should produce a module");
        assert(pyMod->targetLanguage == "python" && "Target should be python");

        // The annotation should still be present in metadata
        auto projectedFns = pyMod->getChildren("functions");
        assert(!projectedFns.empty() && "Should have functions");

        std::cout << "Test 2 PASS: C++ @Deallocate(Explicit) → Python projection" << std::endl;
        ++passed;

        delete var;
        delete anno;
        delete fn;
    }

    // --- Test 3: @Owner(Single) → Python preserves in metadata ---
    {
        Module rustLike("m1", "RustLike", "cpp");
        Function* fn = new Function("f1", "ownership");

        OwnerAnnotation* anno = new OwnerAnnotation();
        anno->id = "a1";
        anno->strategy = "Single";
        fn->addChild("annotations", anno);
        rustLike.addChild("functions", fn);

        auto pyMod = projector.project(&rustLike, "python");

        // @Owner(Single) should be preserved in metadata even though Python doesn't enforce it
        auto projectedFns = pyMod->getChildren("functions");
        auto* projectedFn = static_cast<Function*>(projectedFns[0]);
        auto annos = projectedFn->getChildren("annotations");

        bool hasOwner = false;
        for (auto* a : annos) {
            if (a->conceptType == "OwnerAnnotation") {
                hasOwner = true;
            }
        }
        assert(hasOwner && "@Owner(Single) should be preserved in Python projection metadata");

        std::cout << "Test 3 PASS: @Owner(Single) preserved in Python metadata" << std::endl;
        ++passed;

        delete anno;
        delete fn;
    }

    // --- Test 4: Round-trip preserves annotations ---
    {
        Module pyMod("m1", "RoundTrip", "python");
        Function* fn = new Function("f1", "process");

        ReclaimAnnotation* anno = new ReclaimAnnotation();
        anno->id = "a1";
        anno->strategy = "Tracing";
        fn->addChild("annotations", anno);
        pyMod.addChild("functions", fn);

        // Python → C++
        auto cppMod = projector.project(&pyMod, "cpp");

        // C++ → Python
        auto pyModBack = projector.project(cppMod.get(), "python");

        // Annotation should survive round-trip
        assert(projector.annotationsPreserved(&pyMod, pyModBack.get()) &&
               "Annotations should survive Python → C++ → Python round-trip");

        std::cout << "Test 4 PASS: Round-trip preserves annotations" << std::endl;
        ++passed;

        delete anno;
        delete fn;
    }

    // --- Test 5: @Lifetime(RAII) in C++ generation ---
    {
        Module cppMod("m1", "RAIIModule", "cpp");
        Function* fn = new Function("f1", "scoped");

        LifetimeAnnotation* anno = new LifetimeAnnotation();
        anno->id = "a1";
        anno->strategy = "RAII";
        fn->addChild("annotations", anno);

        Variable* var = new Variable("v1", "resource");
        fn->addChild("body", var);

        PrimitiveType* retType = new PrimitiveType("t1", "void");
        fn->setChild("returnType", retType);
        cppMod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&cppMod);

        // @Lifetime(RAII) → unique_ptr with RAII semantics
        assert(contains(output, "unique_ptr") &&
               "@Lifetime(RAII) should produce unique_ptr in C++");

        std::cout << "Test 5 PASS: @Lifetime(RAII) generates unique_ptr" << std::endl;
        ++passed;

        delete retType;
        delete var;
        delete anno;
        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 66 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
