// Step 108 TDD Test: Batch refactor actions
//
// Tests:
// 1. Rename variable updates declarations and references
// 2. Extract function moves statement and inserts call
// 3. Inline variable removes declaration

#include <cassert>
#include <iostream>
#include "RefactorActions.h"
#include "BatchMutationAPI.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"

static Function* findFunctionByName(Module* mod, const std::string& name) {
    for (auto* child : mod->getChildren("functions")) {
        auto* fn = static_cast<Function*>(child);
        if (fn->name == name) return fn;
    }
    return nullptr;
}

int main() {
    int passed = 0;
    int failed = 0;

    // Test 1: Rename variable
    {
        Module mod("m1", "mod", "python");
        auto* fn = new Function("f1", "foo");
        auto* var = new Variable("v1", "x");
        auto* ref = new VariableReference("r1", "x");
        auto* stmt = new ExpressionStatement();
        stmt->id = "s1";
        stmt->setChild("expression", ref);
        fn->addChild("body", stmt);
        mod.addChild("functions", fn);
        mod.addChild("variables", var);

        auto plan = buildRenameVariablePlan(&mod, "x", "y");
        assert(plan.success);

        BatchMutationAPI batch;
        batch.setRoot(&mod);
        auto res = batch.applySequence(plan.mutations);
        assert(res.success);
        assert(var->name == "y");
        assert(ref->variableName == "y");
        std::cout << "Test 1 PASS: rename variable" << std::endl;
        ++passed;
    }

    // Test 2: Extract function
    {
        Module mod("m2", "mod", "python");
        auto* fn = new Function("f1", "foo");
        auto* ret = new Return();
        ret->id = "r1";
        fn->addChild("body", ret);
        mod.addChild("functions", fn);

        auto plan = buildExtractFunctionPlan(&mod, "extracted");
        assert(plan.success);

        BatchMutationAPI batch;
        batch.setRoot(&mod);
        auto res = batch.applySequence(plan.mutations);
        assert(res.success);

        auto* extracted = findFunctionByName(&mod, "extracted");
        assert(extracted);
        auto* original = findFunctionByName(&mod, "foo");
        assert(original);
        assert(extracted->getChildren("body").size() == 1);
        assert(original->getChildren("body").size() == 1);
        auto* callStmt = original->getChildren("body")[0];
        assert(callStmt->conceptType == "ExpressionStatement");
        auto* callExpr = callStmt->getChild("expression");
        assert(callExpr && callExpr->conceptType == "FunctionCall");
        std::cout << "Test 2 PASS: extract function" << std::endl;
        ++passed;
    }

    // Test 3: Inline variable (remove declaration)
    {
        Module mod("m3", "mod", "python");
        auto* var = new Variable("v1", "z");
        mod.addChild("variables", var);

        auto plan = buildInlineVariablePlan(&mod, "z");
        assert(plan.success);

        BatchMutationAPI batch;
        batch.setRoot(&mod);
        auto res = batch.applySequence(plan.mutations);
        assert(res.success);
        assert(mod.getChildren("variables").empty());
        std::cout << "Test 3 PASS: inline variable" << std::endl;
        ++passed;
    }

    std::cout << "\n=== Step 108 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
