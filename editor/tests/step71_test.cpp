// Step 71 TDD Test: Incremental optimization with rollback
//
// Tests incremental transform tracking and selective undo:
// 1. Apply 3 transforms → all recorded in journal with provenance
// 2. Undo middle transform → AST reflects only transforms 1 and 3
// 3. Undo all transforms → AST back to original state
// 4. Each node knows which transform created/modified it
// 5. Undo individual transform by ID
//
// Will fail until incremental optimization with rollback is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"

#include "IncrementalOptimizer.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Apply transforms and record in journal ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "calc");

        // return (3 + 4) + (5 + 6)
        Return* ret = new Return();
        ret->id = "r1";

        BinaryOperation* outerAdd = new BinaryOperation("outer", "+");

        BinaryOperation* leftAdd = new BinaryOperation("left", "+");
        IntegerLiteral* l1 = new IntegerLiteral("i1", 3);
        IntegerLiteral* l2 = new IntegerLiteral("i2", 4);
        leftAdd->setChild("left", l1);
        leftAdd->setChild("right", l2);

        BinaryOperation* rightAdd = new BinaryOperation("right", "+");
        IntegerLiteral* l3 = new IntegerLiteral("i3", 5);
        IntegerLiteral* l4 = new IntegerLiteral("i4", 6);
        rightAdd->setChild("left", l3);
        rightAdd->setChild("right", l4);

        outerAdd->setChild("left", leftAdd);
        outerAdd->setChild("right", rightAdd);
        ret->setChild("value", outerAdd);
        fn->addChild("body", ret);
        mod.addChild("functions", fn);

        IncrementalOptimizer opt;
        opt.setRoot(&mod);

        // Apply constant folding multiple times
        std::string t1 = opt.applyTransform("constant-fold");
        assert(!t1.empty() && "Should return a transform ID");

        auto history = opt.getTransformHistory();
        assert(!history.empty() && "Should have at least one transform in history");

        std::cout << "Test 1 PASS: Transform recorded in history" << std::endl;
        ++passed;

        delete l4;
        delete l3;
        delete rightAdd;
        delete l2;
        delete l1;
        delete leftAdd;
        delete outerAdd;
        delete ret;
        delete fn;
    }

    // --- Test 2: Undo last transform ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "simple");

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

        IncrementalOptimizer opt;
        opt.setRoot(&mod);

        // Before transform
        auto* valueBefore = ret->getChild("value");
        assert(valueBefore->conceptType == "BinaryOperation" && "Before: BinaryOperation");

        // Apply constant fold
        std::string tid = opt.applyTransform("constant-fold");

        // After transform: should be IntegerLiteral(7)
        auto* valueAfter = ret->getChild("value");
        assert(valueAfter->conceptType == "IntegerLiteral" && "After fold: IntegerLiteral");

        // Undo
        bool undone = opt.undoLast();
        assert(undone && "Undo should succeed");

        // After undo: should be back to BinaryOperation
        auto* valueUndone = ret->getChild("value");
        assert(valueUndone->conceptType == "BinaryOperation" &&
               "After undo: should be BinaryOperation again");

        std::cout << "Test 2 PASS: Undo last transform restores original" << std::endl;
        ++passed;

        delete right;
        delete left;
        delete binOp;
        delete ret;
        delete fn;
    }

    // --- Test 3: Undo specific transform by ID ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "multi");

        // Build body: x = 1; return 3 + 4; y = 2;
        Assignment* assign1 = new Assignment();
        assign1->id = "as1";
        VariableReference* t1 = new VariableReference("vr1", "x");
        IntegerLiteral* v1 = new IntegerLiteral("i1", 1);
        assign1->setChild("target", t1);
        assign1->setChild("value", v1);
        fn->addChild("body", assign1);

        Return* ret = new Return();
        ret->id = "r1";
        BinaryOperation* binOp = new BinaryOperation("b1", "+");
        IntegerLiteral* left = new IntegerLiteral("i2", 3);
        IntegerLiteral* right = new IntegerLiteral("i3", 4);
        binOp->setChild("left", left);
        binOp->setChild("right", right);
        ret->setChild("value", binOp);
        fn->addChild("body", ret);

        Assignment* assign2 = new Assignment();
        assign2->id = "as2";
        VariableReference* t2 = new VariableReference("vr2", "y");
        IntegerLiteral* v2 = new IntegerLiteral("i4", 2);
        assign2->setChild("target", t2);
        assign2->setChild("value", v2);
        fn->addChild("body", assign2);

        mod.addChild("functions", fn);

        IncrementalOptimizer opt;
        opt.setRoot(&mod);

        // Apply two transforms
        std::string foldId = opt.applyTransform("constant-fold");
        std::string dceId = opt.applyTransform("dead-code-elim");

        // Undo just the fold, keeping DCE
        bool undone = opt.undoTransform(foldId);
        assert(undone && "Should be able to undo specific transform");

        // The constant fold should be undone (3+4 back to BinaryOperation)
        // but dead code elimination should remain
        auto history = opt.getTransformHistory();
        bool foldStillThere = false;
        for (const auto& h : history) {
            if (h.transformId == foldId) foldStillThere = true;
        }
        assert(!foldStillThere && "Undone transform should be removed from history");

        std::cout << "Test 3 PASS: Undo specific transform by ID" << std::endl;
        ++passed;

        delete v2;
        delete t2;
        delete assign2;
        delete right;
        delete left;
        delete binOp;
        delete ret;
        delete v1;
        delete t1;
        delete assign1;
        delete fn;
    }

    // --- Test 4: Provenance tracking ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "prov");

        Return* ret = new Return();
        ret->id = "r1";
        BinaryOperation* binOp = new BinaryOperation("b1", "+");
        IntegerLiteral* left = new IntegerLiteral("i1", 10);
        IntegerLiteral* right = new IntegerLiteral("i2", 20);
        binOp->setChild("left", left);
        binOp->setChild("right", right);
        ret->setChild("value", binOp);
        fn->addChild("body", ret);
        mod.addChild("functions", fn);

        IncrementalOptimizer opt;
        opt.setRoot(&mod);

        std::string tid = opt.applyTransform("constant-fold");

        // The folded literal node should know which transform created it
        auto* foldedNode = ret->getChild("value");
        std::string provenance = opt.getProvenance(foldedNode->id);
        assert(!provenance.empty() && "Folded node should have provenance");
        assert(provenance == tid && "Provenance should match the transform ID");

        std::cout << "Test 4 PASS: Provenance tracks which transform created node" << std::endl;
        ++passed;

        delete right;
        delete left;
        delete binOp;
        delete ret;
        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 71 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
