// Step 65 TDD Test: Memory strategy inference
//
// Tests that the system can infer appropriate memory annotations:
// 1. Python source → @Reclaim(Tracing)
// 2. C++ module → at least one suggestion
// 3. Suggestions are not auto-applied
// 4. Suggestions have confidence scores and reasons
// 5. Elisp → @Reclaim(Tracing)

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Annotation.h"
#include "MemoryStrategyInference.h"

static bool hasSuggestion(const std::vector<MemoryStrategyInference::Suggestion>& suggestions,
                           const std::string& annotationType,
                           const std::string& strategy) {
    for (const auto& s : suggestions) {
        if (s.annotationType == annotationType && s.strategy == strategy) {
            return true;
        }
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    MemoryStrategyInference inference;

    // --- Test 1: Python module → @Reclaim(Tracing) suggested ---
    {
        Module mod("m1", "PyModule", "python");
        Function* fn = new Function("f1", "process");
        mod.addChild("functions", fn);

        auto suggestions = inference.inferAnnotations(&mod);
        assert(hasSuggestion(suggestions, "ReclaimAnnotation", "Tracing") &&
               "Python module should suggest @Reclaim(Tracing)");

        std::cout << "Test 1 PASS: Python → @Reclaim(Tracing) suggested" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 2: C++ module with no specific patterns → generic suggestion ---
    {
        Module mod("m1", "CppModule", "cpp");
        Function* fn = new Function("f1", "compute");
        mod.addChild("functions", fn);

        auto suggestions = inference.inferAnnotations(&mod);
        // Should have at least one suggestion
        assert(!suggestions.empty() && "C++ module should have at least one suggestion");

        std::cout << "Test 2 PASS: C++ module gets memory suggestions" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 3: Suggestions are not auto-applied ---
    {
        Module mod("m1", "PyModule", "python");
        Function* fn = new Function("f1", "process");
        mod.addChild("functions", fn);

        // Before inference
        auto annosBefore = fn->getChildren("annotations");
        size_t countBefore = annosBefore.size();

        // Run inference
        inference.inferAnnotations(&mod);

        // After inference — annotations should NOT be added to the AST
        auto annosAfter = fn->getChildren("annotations");
        assert(annosAfter.size() == countBefore &&
               "Inference should NOT auto-apply annotations to AST");

        std::cout << "Test 3 PASS: Suggestions not auto-applied to AST" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 4: Suggestions include confidence scores ---
    {
        Module mod("m1", "PyModule", "python");
        Function* fn = new Function("f1", "process");
        mod.addChild("functions", fn);

        auto suggestions = inference.inferAnnotations(&mod);
        for (const auto& s : suggestions) {
            assert(s.confidence >= 0.0 && s.confidence <= 1.0 &&
                   "Confidence should be between 0 and 1");
            assert(!s.reason.empty() && "Suggestion should have a reason");
        }

        std::cout << "Test 4 PASS: Suggestions have confidence scores and reasons" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 5: Elisp module → @Reclaim(Tracing) suggested ---
    {
        Module mod("m1", "ElModule", "elisp");
        Function* fn = new Function("f1", "process");
        mod.addChild("functions", fn);

        auto suggestions = inference.inferAnnotations(&mod);
        assert(hasSuggestion(suggestions, "ReclaimAnnotation", "Tracing") &&
               "Elisp module should suggest @Reclaim(Tracing)");

        std::cout << "Test 5 PASS: Elisp → @Reclaim(Tracing) suggested" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 65 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
