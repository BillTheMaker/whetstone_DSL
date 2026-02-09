// Step 104 TDD Test: Cross-language projection
//
// Tests:
// 1. Projected module uses target language
// 2. Annotation types preserved across projection

#include <cassert>
#include <iostream>
#include "CrossLanguageProjector.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    Module mod("m1", "mod", "python");
    auto* fn = new Function("f1", "foo");
    auto* anno = new ReclaimAnnotation("a1", "Tracing");
    fn->addChild("annotations", anno);
    mod.addChild("functions", fn);

    CrossLanguageProjector projector;
    auto projected = projector.project(&mod, "cpp");

    assert(projected && projected->targetLanguage == "cpp");
    std::cout << "Test 1 PASS: target language set" << std::endl;
    ++passed;

    assert(projector.annotationsPreserved(&mod, projected.get()));
    std::cout << "Test 2 PASS: annotations preserved" << std::endl;
    ++passed;

    std::cout << "\n=== Step 104 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
