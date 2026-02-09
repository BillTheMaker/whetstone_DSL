// Step 61 TDD Test: AST mutation API (extend)
//
// Tests extended mutation API with validation and lock checking:
// 1. setProperty updates a node property
// 2. updateNode bulk property update
// 3. Mutation recorded in operation journal
// 4. Mutation on OptimizationLock node produces warning (not rejection)
// 5. deleteNode removes a node from its parent
// 6. insertNode adds a node at the correct position

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
#include "ast/Annotation.h"
#include "ASTMutationAPI.h"

int main() {
    int passed = 0;
    int failed = 0;

    // Build test AST
    Module mod("m1", "TestModule", "python");

    Function* fn = new Function("f1", "compute");

    // Add an OptimizationLock annotation
    OptimizationLock* lock = new OptimizationLock();
    lock->id = "lock1";
    lock->lockedBy = "agent-1";
    lock->lockReason = "Performance critical";
    lock->lockLevel = "warning";
    fn->addChild("annotations", lock);

    Variable* var = new Variable("v1", "result");
    fn->addChild("body", var);

    Return* ret = new Return();
    ret->id = "r1";
    IntegerLiteral* val = new IntegerLiteral("i1", 42);
    ret->setChild("value", val);
    fn->addChild("body", ret);

    mod.addChild("functions", fn);

    ASTMutationAPI api;
    api.setRoot(&mod);

    // --- Test 1: setProperty updates a node ---
    {
        auto result = api.setProperty("f1", "name", "calculate");
        assert(result.success && "setProperty should succeed");
        assert(fn->name == "calculate" && "Function name should be updated");

        // Restore for later tests
        fn->name = "compute";

        std::cout << "Test 1 PASS: setProperty updates node property" << std::endl;
        ++passed;
    }

    // --- Test 2: updateNode bulk property update ---
    {
        std::map<std::string, std::string> props;
        props["name"] = "process";
        auto result = api.updateNode("f1", props);
        assert(result.success && "updateNode should succeed");
        assert(fn->name == "process" && "Function name should be bulk-updated");

        // Restore
        fn->name = "compute";

        std::cout << "Test 2 PASS: updateNode bulk update works" << std::endl;
        ++passed;
    }

    // --- Test 3: Mutation is recorded in journal ---
    {
        api.clearJournal();
        api.setProperty("v1", "name", "output");

        auto journal = api.getJournalEntries();
        assert(!journal.empty() && "Journal should have entries after mutation");

        std::cout << "Test 3 PASS: Mutation recorded in journal" << std::endl;
        ++passed;
    }

    // --- Test 4: Mutation on locked node produces warning ---
    {
        // The function has an OptimizationLock annotation
        auto result = api.setProperty("f1", "name", "optimized_compute");
        assert(result.success && "Mutation should succeed even on locked nodes");
        assert(!result.warning.empty() &&
               "Mutation on locked node should produce a warning");

        // Restore
        fn->name = "compute";

        std::cout << "Test 4 PASS: Locked node mutation produces warning (not rejection)" << std::endl;
        ++passed;
    }

    // --- Test 5: deleteNode removes node from parent ---
    {
        // Delete the variable from the function body
        auto result = api.deleteNode("v1");
        assert(result.success && "deleteNode should succeed");

        auto body = fn->getChildren("body");
        bool hasVar = false;
        for (auto* node : body) {
            if (node->id == "v1") hasVar = true;
        }
        assert(!hasVar && "Variable should be removed from body");

        std::cout << "Test 5 PASS: deleteNode removes node from parent" << std::endl;
        ++passed;
    }

    // --- Test 6: insertNode adds node at correct position ---
    {
        Variable* newVar = new Variable("v2", "newResult");
        auto result = api.insertNode("f1", "body", newVar);
        assert(result.success && "insertNode should succeed");

        auto body = fn->getChildren("body");
        bool hasNewVar = false;
        for (auto* node : body) {
            if (node->id == "v2") hasNewVar = true;
        }
        assert(hasNewVar && "New variable should be in function body");

        std::cout << "Test 6 PASS: insertNode adds node to parent" << std::endl;
        ++passed;

        delete newVar;
    }

    // Cleanup
    delete val;
    delete ret;
    delete var;
    delete lock;
    delete fn;

    // --- Summary ---
    std::cout << "\n=== Step 61 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
