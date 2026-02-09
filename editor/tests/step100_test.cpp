// Step 100 TDD Test: Memory strategy suggestions
//
// Tests:
// 1. Inference suggests tracing for Python module

#include <cassert>
#include <iostream>
#include "MemoryStrategyInference.h"
#include "ast/Module.h"

int main() {
    int passed = 0;
    int failed = 0;

    Module mod("m1", "mod", "python");
    MemoryStrategyInference inf;
    auto suggestions = inf.inferAnnotations(&mod);
    bool found = false;
    for (const auto& s : suggestions) {
        if (s.annotationType == "ReclaimAnnotation" && s.strategy == "Tracing" && s.confidence > 0.5) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "Test 1 PASS: tracing suggestion for python" << std::endl;
    ++passed;

    std::cout << "\n=== Step 100 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
