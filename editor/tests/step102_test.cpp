// Step 102 TDD Test: Memory strategy dashboard helpers
//
// Tests:
// 1. Collect annotation entries and build JSON

#include <cassert>
#include <iostream>
#include "StrategyDashboard.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    Module* mod = new Module("m1", "mod", "cpp");
    mod->setSpan(0, 0, 5, 0);
    auto* fn = new Function("f1", "foo");
    fn->setSpan(1, 0, 3, 0);
    mod->addChild("functions", fn);

    auto* anno = new LifetimeAnnotation("a1", "RAII");
    fn->addChild("annotations", anno);

    std::vector<AnnotationEntry> entries;
    collectAnnotationEntries(mod, entries);
    assert(entries.size() == 1);
    auto j = buildAnnotationSummaryJson(entries);
    assert(j.contains("annotations"));
    assert(j["annotations"].size() == 1);
    std::cout << "Test 1 PASS: dashboard helpers" << std::endl;
    ++passed;

    std::cout << "\n=== Step 102 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
