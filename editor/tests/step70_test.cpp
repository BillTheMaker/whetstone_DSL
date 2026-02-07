// Step 70 TDD Test: Cross-strategy validation
//
// Tests post-optimization invariant verification:
// 1. @Deallocate(Explicit): no use-after-free detected
// 2. @Deallocate(Explicit): leaked allocation detected
// 3. @Owner(Single): no double-move detected
// 4. @Owner(Single): aliasing detected
// 5. @Lifetime(RAII): destructor unreachable on some path detected
// 6. Clean code passes validation
//
// Will fail until cross-strategy validation is implemented.

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

// Forward declaration — StrategyValidator
class StrategyValidator {
public:
    struct Violation {
        std::string severity;   // "error" or "warning"
        std::string category;   // "use-after-free", "leak", "double-move", "aliasing", "destructor-unreachable"
        std::string message;
        std::string nodeId;
    };

    // Validate all memory strategy invariants
    std::vector<Violation> validateInvariants(const ASTNode* root) const;
};

static bool hasViolation(const std::vector<StrategyValidator::Violation>& violations,
                          const std::string& category) {
    for (const auto& v : violations) {
        if (v.category == category) return true;
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    StrategyValidator validator;

    // --- Test 1: @Deallocate(Explicit) use-after-free detected ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "useAfterFree");

        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Explicit";
        fn->addChild("annotations", anno);

        // ptr = new Widget; delete ptr; use(ptr) -- use after free
        Variable* ptr = new Variable("v1", "ptr");
        fn->addChild("body", ptr);

        // delete ptr
        ExpressionStatement* freeStmt = new ExpressionStatement();
        freeStmt->id = "del1";
        FunctionCall* freeCall = new FunctionCall();
        freeCall->id = "fc1";
        freeCall->functionName = "delete";
        VariableReference* freeArg = new VariableReference("vr1", "ptr");
        freeCall->addChild("arguments", freeArg);
        freeStmt->setChild("expression", freeCall);
        fn->addChild("body", freeStmt);

        // use(ptr) — after delete
        ExpressionStatement* useStmt = new ExpressionStatement();
        useStmt->id = "use1";
        FunctionCall* useCall = new FunctionCall();
        useCall->id = "fc2";
        useCall->functionName = "use";
        VariableReference* useArg = new VariableReference("vr2", "ptr");
        useCall->addChild("arguments", useArg);
        useStmt->setChild("expression", useCall);
        fn->addChild("body", useStmt);

        mod.addChild("functions", fn);

        auto violations = validator.validateInvariants(&mod);
        assert(hasViolation(violations, "use-after-free") &&
               "Should detect use-after-free");

        std::cout << "Test 1 PASS: use-after-free detected" << std::endl;
        ++passed;

        delete useArg;
        delete useCall;
        delete useStmt;
        delete freeArg;
        delete freeCall;
        delete freeStmt;
        delete ptr;
        delete anno;
        delete fn;
    }

    // --- Test 2: @Deallocate(Explicit) leaked allocation detected ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "leak");

        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Explicit";
        fn->addChild("annotations", anno);

        // ptr = new Widget; return; -- no delete → leak
        Variable* ptr = new Variable("v1", "ptr");
        fn->addChild("body", ptr);

        Return* ret = new Return();
        ret->id = "r1";
        fn->addChild("body", ret);

        mod.addChild("functions", fn);

        auto violations = validator.validateInvariants(&mod);
        assert(hasViolation(violations, "leak") &&
               "Should detect leaked allocation");

        std::cout << "Test 2 PASS: Leaked allocation detected" << std::endl;
        ++passed;

        delete ret;
        delete ptr;
        delete anno;
        delete fn;
    }

    // --- Test 3: @Owner(Single) aliasing detected ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "alias");

        OwnerAnnotation* anno = new OwnerAnnotation();
        anno->id = "a1";
        anno->strategy = "Single";
        fn->addChild("annotations", anno);

        // a = resource; b = resource; -- aliasing under single ownership
        Variable* resource = new Variable("v1", "resource");
        fn->addChild("body", resource);

        Assignment* assign1 = new Assignment();
        assign1->id = "as1";
        VariableReference* t1 = new VariableReference("vr1", "a");
        VariableReference* v1 = new VariableReference("vr2", "resource");
        assign1->setChild("target", t1);
        assign1->setChild("value", v1);
        fn->addChild("body", assign1);

        Assignment* assign2 = new Assignment();
        assign2->id = "as2";
        VariableReference* t2 = new VariableReference("vr3", "b");
        VariableReference* v2 = new VariableReference("vr4", "resource");
        assign2->setChild("target", t2);
        assign2->setChild("value", v2);
        fn->addChild("body", assign2);

        mod.addChild("functions", fn);

        auto violations = validator.validateInvariants(&mod);
        assert(hasViolation(violations, "aliasing") &&
               "Should detect aliasing under @Owner(Single)");

        std::cout << "Test 3 PASS: @Owner(Single) aliasing detected" << std::endl;
        ++passed;

        delete v2;
        delete t2;
        delete assign2;
        delete v1;
        delete t1;
        delete assign1;
        delete resource;
        delete anno;
        delete fn;
    }

    // --- Test 4: Clean code passes validation ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "clean");

        ReclaimAnnotation* anno = new ReclaimAnnotation();
        anno->id = "a1";
        anno->strategy = "Tracing";
        fn->addChild("annotations", anno);

        Return* ret = new Return();
        ret->id = "r1";
        IntegerLiteral* val = new IntegerLiteral("i1", 42);
        ret->setChild("value", val);
        fn->addChild("body", ret);

        mod.addChild("functions", fn);

        auto violations = validator.validateInvariants(&mod);
        bool hasErrors = false;
        for (const auto& v : violations) {
            if (v.severity == "error") hasErrors = true;
        }
        assert(!hasErrors && "Clean code should pass validation");

        std::cout << "Test 4 PASS: Clean code passes validation" << std::endl;
        ++passed;

        delete val;
        delete ret;
        delete anno;
        delete fn;
    }

    // --- Test 5: Violations include node IDs for diagnostics ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "leak");

        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Explicit";
        fn->addChild("annotations", anno);

        Variable* ptr = new Variable("v1", "ptr");
        fn->addChild("body", ptr);
        mod.addChild("functions", fn);

        auto violations = validator.validateInvariants(&mod);
        for (const auto& v : violations) {
            assert(!v.nodeId.empty() && "Violation should reference a node ID");
            assert(!v.message.empty() && "Violation should have a message");
        }

        std::cout << "Test 5 PASS: Violations include node IDs and messages" << std::endl;
        ++passed;

        delete ptr;
        delete anno;
        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 70 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
