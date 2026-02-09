// Step 63 TDD Test: Batch operations (extend)
//
// Tests transactional batch mutations:
// 1. applySequence of mutations applies all atomically
// 2. If one mutation in the batch fails, all preceding mutations roll back
// 3. Successful batch records all operations in journal
// 4. Empty batch succeeds trivially
// 5. Partial failure doesn't leave AST in inconsistent state

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <map>
#include <functional>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Expression.h"
#include "BatchMutationAPI.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Successful batch of 3 mutations ---
    {
        Module mod("m1", "TestModule", "python");
        Function* fn1 = new Function("f1", "alpha");
        Function* fn2 = new Function("f2", "beta");
        Function* fn3 = new Function("f3", "gamma");
        mod.addChild("functions", fn1);
        mod.addChild("functions", fn2);
        mod.addChild("functions", fn3);

        BatchMutationAPI api;
        api.setRoot(&mod);

        std::vector<BatchMutationAPI::Mutation> mutations;
        mutations.push_back({"setProperty", "f1", "name", "ALPHA", "", "", nullptr});
        mutations.push_back({"setProperty", "f2", "name", "BETA", "", "", nullptr});
        mutations.push_back({"setProperty", "f3", "name", "GAMMA", "", "", nullptr});

        auto result = api.applySequence(mutations);
        assert(result.success && "Batch should succeed");
        assert(result.appliedCount == 3 && "All 3 mutations should apply");

        assert(fn1->name == "ALPHA" && "fn1 should be renamed");
        assert(fn2->name == "BETA" && "fn2 should be renamed");
        assert(fn3->name == "GAMMA" && "fn3 should be renamed");

        std::cout << "Test 1 PASS: Batch of 3 mutations all applied" << std::endl;
        ++passed;

        delete fn3;
        delete fn2;
        delete fn1;
    }

    // --- Test 2: Failed batch rolls back all preceding mutations ---
    {
        Module mod("m1", "TestModule", "python");
        Function* fn1 = new Function("f1", "alpha");
        Function* fn2 = new Function("f2", "beta");
        mod.addChild("functions", fn1);
        mod.addChild("functions", fn2);

        BatchMutationAPI api;
        api.setRoot(&mod);

        std::vector<BatchMutationAPI::Mutation> mutations;
        mutations.push_back({"setProperty", "f1", "name", "ALPHA", "", "", nullptr});
        mutations.push_back({"setProperty", "f2", "name", "BETA", "", "", nullptr});
        // Third mutation targets a non-existent node — should fail
        mutations.push_back({"setProperty", "f_nonexistent", "name", "NOPE", "", "", nullptr});

        auto result = api.applySequence(mutations);
        assert(!result.success && "Batch should fail due to bad node ID");
        assert(!result.error.empty() && "Should have an error message");

        // ALL preceding mutations should be rolled back
        assert(fn1->name == "alpha" && "fn1 should be rolled back to 'alpha'");
        assert(fn2->name == "beta" && "fn2 should be rolled back to 'beta'");

        std::cout << "Test 2 PASS: Failed batch rolls back all preceding mutations" << std::endl;
        ++passed;

        delete fn2;
        delete fn1;
    }

    // --- Test 3: Successful batch records in journal ---
    {
        Module mod("m1", "TestModule", "python");
        Function* fn = new Function("f1", "alpha");
        mod.addChild("functions", fn);

        BatchMutationAPI api;
        api.setRoot(&mod);
        api.clearJournal();

        std::vector<BatchMutationAPI::Mutation> mutations;
        mutations.push_back({"setProperty", "f1", "name", "ALPHA", "", "", nullptr});

        api.applySequence(mutations);
        auto journal = api.getJournalEntries();
        assert(!journal.empty() && "Journal should record batch operations");

        std::cout << "Test 3 PASS: Successful batch recorded in journal" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 4: Empty batch succeeds trivially ---
    {
        Module mod("m1", "TestModule", "python");

        BatchMutationAPI api;
        api.setRoot(&mod);

        std::vector<BatchMutationAPI::Mutation> mutations;  // empty
        auto result = api.applySequence(mutations);
        assert(result.success && "Empty batch should succeed");
        assert(result.appliedCount == 0 && "No mutations to apply");

        std::cout << "Test 4 PASS: Empty batch succeeds trivially" << std::endl;
        ++passed;
    }

    // --- Test 5: Batch with 5 mutations, error at step 3, all 5 rolled back ---
    {
        Module mod("m1", "TestModule", "python");
        Function* fn1 = new Function("f1", "a");
        Function* fn2 = new Function("f2", "b");
        Variable* var = new Variable("v1", "c");
        mod.addChild("functions", fn1);
        mod.addChild("functions", fn2);
        mod.addChild("variables", var);

        BatchMutationAPI api;
        api.setRoot(&mod);

        std::vector<BatchMutationAPI::Mutation> mutations;
        mutations.push_back({"setProperty", "f1", "name", "A", "", "", nullptr});
        mutations.push_back({"setProperty", "f2", "name", "B", "", "", nullptr});
        mutations.push_back({"setProperty", "bad_id", "name", "FAIL", "", "", nullptr});  // FAIL
        mutations.push_back({"setProperty", "v1", "name", "C", "", "", nullptr});
        mutations.push_back({"setProperty", "f1", "name", "AA", "", "", nullptr});

        auto result = api.applySequence(mutations);
        assert(!result.success && "Batch should fail at step 3");

        // Everything should be rolled back
        assert(fn1->name == "a" && "fn1 should be rolled back");
        assert(fn2->name == "b" && "fn2 should be rolled back");
        assert(var->name == "c" && "var should be unchanged");

        std::cout << "Test 5 PASS: 5 mutations, error at 3, all rolled back" << std::endl;
        ++passed;

        delete var;
        delete fn2;
        delete fn1;
    }

    // --- Summary ---
    std::cout << "\n=== Step 63 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
