// Step 101 TDD Test: Annotation conflict detection
//
// Tests:
// 1. Conflict detected between parent/child annotations

#include <cassert>
#include <iostream>
#include "AnnotationConflict.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    auto* mod = new Module("m1", "mod", "cpp");
    mod->setSpan(0, 0, 10, 0);
    auto* fn = new Function("f1", "foo");
    fn->setSpan(2, 0, 5, 0);
    mod->addChild("functions", fn);

    auto* parentAnno = new OwnerAnnotation("a1", "Shared_ARC");
    auto* childAnno = new OwnerAnnotation("a2", "Single");
    mod->addChild("annotations", parentAnno);
    fn->addChild("annotations", childAnno);

    std::vector<AnnotationConflict> conflicts;
    collectAnnotationConflicts(mod, conflicts);
    assert(conflicts.size() == 1);
    assert(conflicts[0].childAnnoId == "a2");
    assert(conflicts[0].parentAnnoId == "a1");
    std::cout << "Test 1 PASS: conflict detected" << std::endl;
    ++passed;

    std::cout << "\n=== Step 101 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
