// Step 105 TDD Test: Optimization controls
//
// Tests:
// 1. StrategyAwareOptimizer blocks optimization when @Owner(Single) present
// 2. TransformEngine constant folding modifies nodes

#include <cassert>
#include <iostream>
#include "StrategyAwareOptimizer.h"
#include "TransformEngine.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;
    int failed = 0;

    Module mod("m1", "mod", "python");
    auto* fn = new Function("f1", "foo");
    auto* owner = new OwnerAnnotation("a1", "Single");
    fn->addChild("annotations", owner);
    mod.addChild("functions", fn);

    StrategyAwareOptimizer opt;
    opt.setRoot(&mod);
    auto res = opt.optimizeFunction("f1");
    assert(!res.blocked.empty());
    std::cout << "Test 1 PASS: optimization blocked by annotation" << std::endl;
    ++passed;

    Module mod2("m2", "mod2", "python");
    auto* fn2 = new Function("f2", "bar");
    auto* ret = new Return();
    ret->id = "r1";
    auto* bin = new BinaryOperation("b1", "+");
    auto* left = new IntegerLiteral("i1", 2);
    auto* right = new IntegerLiteral("i2", 3);
    bin->setChild("left", left);
    bin->setChild("right", right);
    ret->setChild("value", bin);
    fn2->addChild("body", ret);
    mod2.addChild("functions", fn2);

    TransformEngine engine;
    engine.setRoot(&mod2);
    auto foldRes = engine.constantFolding();
    assert(foldRes.applied && foldRes.nodesModified > 0);
    std::cout << "Test 2 PASS: constant folding applied" << std::endl;
    ++passed;

    std::cout << "\n=== Step 105 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
