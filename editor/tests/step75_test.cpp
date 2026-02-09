// Step 75 TDD Test: API documentation and surface verification
//
// Tests:
// 1. APIDocGenerator produces valid markdown documentation
// 2. All major API categories are covered (parser, generator, validator, optimizer, editor, api)
// 3. Each entry has component name, description, and methods
// 4. Documentation includes all core components (at least 20)
// 5. Generated markdown contains expected section headers
// 6. Pipeline API is documented

#include <iostream>
#include <string>
#include <cassert>
#include "APIDocGenerator.h"

int main() {
    int passed = 0;
    int failed = 0;
    APIDocGenerator docGen;

    // --- Test 1: APIDocGenerator produces valid markdown ---
    {
        std::string markdown = docGen.generateMarkdown();
        assert(!markdown.empty() && "Should produce non-empty markdown");
        assert(markdown.find("# Whetstone DSL API Reference") != std::string::npos &&
               "Should have title");
        assert(markdown.find("##") != std::string::npos &&
               "Should have section headers");

        std::cout << "Test 1 PASS: APIDocGenerator produces valid markdown" << std::endl;
        ++passed;
    }

    // --- Test 2: All major API categories covered ---
    {
        bool covered = docGen.verifyCoverage();
        assert(covered && "All API categories should be covered");

        std::cout << "Test 2 PASS: All major API categories covered" << std::endl;
        ++passed;
    }

    // --- Test 3: Each entry has required fields ---
    {
        auto entries = docGen.getAPIEntries();
        for (const auto& e : entries) {
            assert(!e.component.empty() && "Entry must have component name");
            assert(!e.category.empty() && "Entry must have category");
            assert(!e.description.empty() && "Entry must have description");
            assert(!e.methods.empty() && "Entry must have at least one method");
        }

        std::cout << "Test 3 PASS: All entries have required fields" << std::endl;
        ++passed;
    }

    // --- Test 4: Documentation covers at least 20 components ---
    {
        auto entries = docGen.getAPIEntries();
        assert(entries.size() >= 20 && "Should document at least 20 components");

        std::cout << "Test 4 PASS: Documents " << entries.size() << " components (>= 20)" << std::endl;
        ++passed;
    }

    // --- Test 5: Markdown contains expected category sections ---
    {
        std::string markdown = docGen.generateMarkdown();
        assert(markdown.find("## Parser") != std::string::npos && "Should have Parser section");
        assert(markdown.find("## Generator") != std::string::npos && "Should have Generator section");
        assert(markdown.find("## Validator") != std::string::npos && "Should have Validator section");
        assert(markdown.find("## Optimizer") != std::string::npos && "Should have Optimizer section");
        assert(markdown.find("## Editor") != std::string::npos && "Should have Editor section");

        std::cout << "Test 5 PASS: Markdown contains expected category sections" << std::endl;
        ++passed;
    }

    // --- Test 6: Core components are documented ---
    {
        auto entries = docGen.getAPIEntries();
        bool hasPipeline = false, hasTransformEngine = false, hasTreeSitter = false;
        bool hasStrategyValidator = false, hasIncrementalOpt = false;

        for (const auto& e : entries) {
            if (e.component == "Pipeline") hasPipeline = true;
            if (e.component == "TransformEngine") hasTransformEngine = true;
            if (e.component == "TreeSitterParser") hasTreeSitter = true;
            if (e.component == "StrategyValidator") hasStrategyValidator = true;
            if (e.component == "IncrementalOptimizer") hasIncrementalOpt = true;
        }

        assert(hasPipeline && "Pipeline should be documented");
        assert(hasTransformEngine && "TransformEngine should be documented");
        assert(hasTreeSitter && "TreeSitterParser should be documented");
        assert(hasStrategyValidator && "StrategyValidator should be documented");
        assert(hasIncrementalOpt && "IncrementalOptimizer should be documented");

        std::cout << "Test 6 PASS: Core components are documented" << std::endl;
        ++passed;
    }

    // --- Print generated documentation ---
    std::cout << "\n--- Generated API Documentation Preview ---\n" << std::endl;
    std::string markdown = docGen.generateMarkdown();
    // Print first 1000 chars as preview
    std::cout << markdown.substr(0, 1000);
    if (markdown.size() > 1000) std::cout << "\n... (" << markdown.size() << " total chars)\n";
    std::cout << std::endl;

    // --- Summary ---
    std::cout << "=== Step 75 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
