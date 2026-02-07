// Step 68 TDD Test: Transformation engine
//
// Tests AST transformation framework:
// 1. Constant folding: 3 + 4 → 7 in AST
// 2. Dead code elimination: unreachable code after return removed
// 3. Transform checks OptimizationLock before modifying (warn, don't reject)
// 4. Respect @Loop annotations
// 5. Transform framework defines pre/post conditions
//
// Will fail until the transformation engine is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <memory>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"

// Forward declaration — TransformEngine
class TransformEngine {
public:
    struct TransformResult {
        bool applied;
        std::string warning;  // Non-empty if warning produced
        int nodesModified;
    };

    // Set the AST root to transform
    void setRoot(ASTNode* root);

    // Apply constant folding pass
    TransformResult constantFolding();

    // Apply dead code elimination pass
    TransformResult deadCodeElimination();

    // Apply all built-in transforms
    std::vector<TransformResult> applyAll();
};

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Constant folding 3 + 4 → 7 ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "calc");

        // return 3 + 4
        Return* ret = new Return();
        ret->id = "r1";
        BinaryOperation* binOp = new BinaryOperation("b1", "+");
        IntegerLiteral* left = new IntegerLiteral("i1", 3);
        IntegerLiteral* right = new IntegerLiteral("i2", 4);
        binOp->setChild("left", left);
        binOp->setChild("right", right);
        ret->setChild("value", binOp);
        fn->addChild("body", ret);
        mod.addChild("functions", fn);

        TransformEngine engine;
        engine.setRoot(&mod);

        auto result = engine.constantFolding();
        assert(result.applied && "Constant folding should apply");
        assert(result.nodesModified > 0 && "Should modify at least one node");

        // After folding, the return value should be IntegerLiteral(7)
        auto* newValue = ret->getChild("value");
        assert(newValue != nullptr && "Return should still have a value");
        assert(newValue->conceptType == "IntegerLiteral" &&
               "Folded result should be IntegerLiteral");
        auto* foldedLit = static_cast<IntegerLiteral*>(newValue);
        assert(foldedLit->value == 7 && "3 + 4 should fold to 7");

        std::cout << "Test 1 PASS: Constant folding 3 + 4 → 7" << std::endl;
        ++passed;

        delete right;
        delete left;
        delete binOp;
        delete ret;
        delete fn;
    }

    // --- Test 2: Dead code elimination removes code after return ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "early");

        // return 1; x = 2 (dead code)
        Return* ret = new Return();
        ret->id = "r1";
        IntegerLiteral* retVal = new IntegerLiteral("i1", 1);
        ret->setChild("value", retVal);
        fn->addChild("body", ret);

        Assignment* deadAssign = new Assignment();
        deadAssign->id = "dead1";
        VariableReference* target = new VariableReference("vr1", "x");
        IntegerLiteral* val = new IntegerLiteral("i2", 2);
        deadAssign->setChild("target", target);
        deadAssign->setChild("value", val);
        fn->addChild("body", deadAssign);

        mod.addChild("functions", fn);

        TransformEngine engine;
        engine.setRoot(&mod);

        auto result = engine.deadCodeElimination();
        assert(result.applied && "DCE should apply");

        // The assignment after return should be removed
        auto body = fn->getChildren("body");
        assert(body.size() == 1 && "Only the return should remain");
        assert(body[0]->conceptType == "Return" && "Remaining stmt should be Return");

        std::cout << "Test 2 PASS: Dead code after return eliminated" << std::endl;
        ++passed;

        delete val;
        delete target;
        delete deadAssign;
        delete retVal;
        delete ret;
        delete fn;
    }

    // --- Test 3: Locked node produces warning but still transforms ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "locked");

        OptimizationLock* lock = new OptimizationLock();
        lock->id = "lock1";
        lock->lockedBy = "user";
        lock->lockReason = "Don't optimize";
        lock->lockLevel = "warning";
        fn->addChild("annotations", lock);

        // return 5 + 3
        Return* ret = new Return();
        ret->id = "r1";
        BinaryOperation* binOp = new BinaryOperation("b1", "+");
        IntegerLiteral* left = new IntegerLiteral("i1", 5);
        IntegerLiteral* right = new IntegerLiteral("i2", 3);
        binOp->setChild("left", left);
        binOp->setChild("right", right);
        ret->setChild("value", binOp);
        fn->addChild("body", ret);
        mod.addChild("functions", fn);

        TransformEngine engine;
        engine.setRoot(&mod);

        auto result = engine.constantFolding();
        assert(result.applied && "Should still apply (warning level, not block)");
        assert(!result.warning.empty() && "Should produce a warning for locked node");

        // Should still fold
        auto* newValue = ret->getChild("value");
        assert(newValue->conceptType == "IntegerLiteral" && "Should still fold");
        auto* lit = static_cast<IntegerLiteral*>(newValue);
        assert(lit->value == 8 && "5 + 3 should fold to 8");

        std::cout << "Test 3 PASS: Locked node warns but still transforms" << std::endl;
        ++passed;

        delete right;
        delete left;
        delete binOp;
        delete ret;
        delete lock;
        delete fn;
    }

    // --- Test 4: No transformation needed → result.applied is false ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "simple");

        // return x (nothing to fold)
        Return* ret = new Return();
        ret->id = "r1";
        VariableReference* ref = new VariableReference("vr1", "x");
        ret->setChild("value", ref);
        fn->addChild("body", ret);
        mod.addChild("functions", fn);

        TransformEngine engine;
        engine.setRoot(&mod);

        auto result = engine.constantFolding();
        assert(!result.applied && "No constants to fold");
        assert(result.nodesModified == 0 && "No nodes should be modified");

        std::cout << "Test 4 PASS: No-op when nothing to transform" << std::endl;
        ++passed;

        delete ref;
        delete ret;
        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 68 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
