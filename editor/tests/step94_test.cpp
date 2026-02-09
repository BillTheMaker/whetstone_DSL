// Step 94 TDD Test: Whetstone diagnostics aggregation
//
// Tests:
// 1. Whetstone diagnostics are tagged and mapped to editor diagnostics

#include <cassert>
#include <iostream>
#include "Diagnostics.h"

int main() {
    int passed = 0;
    int failed = 0;

    std::vector<AnnotationValidator::Diagnostic> annos = {
        {"error", "Missing Intent", "node1"}
    };
    std::vector<StrategyValidator::Violation> violations = {
        {"warning", "leak", "Leak detected", "node2"}
    };

    auto out = collectWhetstoneDiagnostics(annos, violations, "file:///tmp/a.py");
    assert(out.size() == 2);
    assert(out[0].severity == 1);
    assert(out[0].message.rfind("[Whetstone]", 0) == 0);
    assert(out[1].severity == 2);
    assert(out[1].message.rfind("[Whetstone]", 0) == 0);
    std::cout << "Test 1 PASS: Whetstone diagnostics aggregated" << std::endl;
    ++passed;

    std::cout << "\n=== Step 94 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
