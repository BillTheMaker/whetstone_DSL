// Step 106 TDD Test: Transform history timestamps
//
// Tests:
// 1. Transform history records timestamp entries

#include <cassert>
#include <iostream>
#include "IncrementalOptimizer.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Statement.h"
#include "ast/Expression.h"

int main() {
    int passed = 0;
    int failed = 0;

    Module mod("m1", "mod", "python");
    auto* fn = new Function("f1", "foo");
    auto* ret = new Return();
    ret->id = "r1";
    auto* bin = new BinaryOperation("b1", "+");
    auto* left = new IntegerLiteral("i1", 1);
    auto* right = new IntegerLiteral("i2", 2);
    bin->setChild("left", left);
    bin->setChild("right", right);
    ret->setChild("value", bin);
    fn->addChild("body", ret);
    mod.addChild("functions", fn);

    IncrementalOptimizer opt;
    opt.setRoot(&mod);
    opt.applyTransform("constant-fold");

    auto history = opt.getTransformHistory();
    assert(!history.empty());
    assert(!history[0].timestamp.empty());

    std::cout << "Test 1 PASS: timestamps recorded" << std::endl;
    ++passed;

    std::cout << "\n=== Step 106 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
