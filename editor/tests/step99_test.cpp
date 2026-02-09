// Step 99 TDD Test: Annotation context menu mutations
//
// Tests:
// 1. ASTMutationAPI can add and remove annotation nodes

#include <cassert>
#include <iostream>
#include "ASTMutationAPI.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    Module* mod = new Module("m1", "mod", "python");
    auto* fn = new Function("f1", "foo");
    mod->addChild("functions", fn);

    ASTMutationAPI api;
    api.setRoot(mod);

    auto* anno = new ReclaimAnnotation("a1", "Tracing");
    auto res = api.insertNode(fn->id, "annotations", anno);
    assert(res.success);
    assert(fn->getChildren("annotations").size() == 1);
    std::cout << "Test 1 PASS: annotation inserted" << std::endl;
    ++passed;

    auto res2 = api.deleteNode("a1");
    assert(res2.success);
    assert(fn->getChildren("annotations").empty());
    std::cout << "Test 2 PASS: annotation removed" << std::endl;
    ++passed;

    std::cout << "\n=== Step 99 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
