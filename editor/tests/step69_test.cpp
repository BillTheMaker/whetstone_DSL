// Step 69 TDD Test: Strategy-aware optimization
//
// Tests that transforms respect memory annotations:
// 1. @Owner(Single) → can inline/move but not duplicate (no aliasing)
// 2. @Deallocate(Explicit) → cannot reorder around deallocation points
// 3. @Reclaim(Tracing) → can freely restructure (GC handles cleanup)
// 4. @Allocate(Static) → cannot dynamically allocate
//
// Will fail until strategy-aware optimization is implemented.

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

// Forward declaration — StrategyAwareOptimizer
class StrategyAwareOptimizer {
public:
    struct OptResult {
        bool applied;
        std::string warning;
        std::string blocked;   // Non-empty if optimization was blocked by annotation
        int nodesModified;
    };

    void setRoot(ASTNode* root);

    // Try to inline a variable (replace references with the value)
    OptResult inlineVariable(const std::string& variableId);

    // Try to reorder statements for efficiency
    OptResult reorderStatements(const std::string& functionId);

    // Try to duplicate/clone a node for parallelism
    OptResult duplicateNode(const std::string& nodeId);

    // General optimization respecting all annotations
    OptResult optimizeFunction(const std::string& functionId);
};

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: @Reclaim(Tracing) allows free restructuring ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "process");

        ReclaimAnnotation* anno = new ReclaimAnnotation();
        anno->id = "a1";
        anno->strategy = "Tracing";
        fn->addChild("annotations", anno);

        Variable* var = new Variable("v1", "temp");
        fn->addChild("body", var);

        Assignment* assign = new Assignment();
        assign->id = "as1";
        VariableReference* target = new VariableReference("vr1", "result");
        VariableReference* value = new VariableReference("vr2", "temp");
        assign->setChild("target", target);
        assign->setChild("value", value);
        fn->addChild("body", assign);

        mod.addChild("functions", fn);

        StrategyAwareOptimizer opt;
        opt.setRoot(&mod);

        auto result = opt.optimizeFunction("f1");
        assert(result.blocked.empty() &&
               "@Reclaim(Tracing) should not block any optimization");

        std::cout << "Test 1 PASS: @Reclaim(Tracing) allows free restructuring" << std::endl;
        ++passed;

        delete value;
        delete target;
        delete assign;
        delete var;
        delete anno;
        delete fn;
    }

    // --- Test 2: @Owner(Single) blocks duplication/aliasing ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "ownership");

        OwnerAnnotation* anno = new OwnerAnnotation();
        anno->id = "a1";
        anno->strategy = "Single";
        fn->addChild("annotations", anno);

        Variable* var = new Variable("v1", "resource");
        fn->addChild("body", var);
        mod.addChild("functions", fn);

        StrategyAwareOptimizer opt;
        opt.setRoot(&mod);

        // Try to duplicate a node under single ownership — should be blocked
        auto result = opt.duplicateNode("v1");
        assert(!result.blocked.empty() &&
               "@Owner(Single) should block duplication (aliasing)");

        std::cout << "Test 2 PASS: @Owner(Single) blocks duplication" << std::endl;
        ++passed;

        delete var;
        delete anno;
        delete fn;
    }

    // --- Test 3: @Deallocate(Explicit) blocks reordering around dealloc ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "manualMem");

        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Explicit";
        fn->addChild("annotations", anno);

        // Use then free pattern — reordering could cause use-after-free
        Variable* ptr = new Variable("v1", "ptr");
        fn->addChild("body", ptr);

        ExpressionStatement* use = new ExpressionStatement();
        use->id = "use1";
        FunctionCall* call = new FunctionCall();
        call->id = "fc1";
        call->functionName = "process";
        VariableReference* arg = new VariableReference("vr1", "ptr");
        call->addChild("arguments", arg);
        use->setChild("expression", call);
        fn->addChild("body", use);

        // "delete ptr" — simulated as function call
        ExpressionStatement* freeStmt = new ExpressionStatement();
        freeStmt->id = "free1";
        FunctionCall* freeCall = new FunctionCall();
        freeCall->id = "fc2";
        freeCall->functionName = "delete";
        VariableReference* freeArg = new VariableReference("vr2", "ptr");
        freeCall->addChild("arguments", freeArg);
        freeStmt->setChild("expression", freeCall);
        fn->addChild("body", freeStmt);

        mod.addChild("functions", fn);

        StrategyAwareOptimizer opt;
        opt.setRoot(&mod);

        auto result = opt.reorderStatements("f1");
        assert(!result.blocked.empty() &&
               "@Deallocate(Explicit) should block reordering around deallocation");

        std::cout << "Test 3 PASS: @Deallocate(Explicit) blocks unsafe reordering" << std::endl;
        ++passed;

        delete freeArg;
        delete freeCall;
        delete freeStmt;
        delete arg;
        delete call;
        delete use;
        delete ptr;
        delete anno;
        delete fn;
    }

    // --- Test 4: @Allocate(Static) blocks dynamic allocation ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "staticOnly");

        AllocateAnnotation* anno = new AllocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Static";
        fn->addChild("annotations", anno);

        Variable* var = new Variable("v1", "buffer");
        fn->addChild("body", var);
        mod.addChild("functions", fn);

        StrategyAwareOptimizer opt;
        opt.setRoot(&mod);

        // An optimization that would require dynamic allocation should be blocked
        auto result = opt.duplicateNode("v1");
        assert(!result.blocked.empty() &&
               "@Allocate(Static) should block dynamic allocation");

        std::cout << "Test 4 PASS: @Allocate(Static) blocks dynamic allocation" << std::endl;
        ++passed;

        delete var;
        delete anno;
        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 69 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
