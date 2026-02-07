// Step 62 TDD Test: Context API
//
// Tests the agent context API:
// 1. getInScopeSymbols returns variables/functions/parameters visible at a position
// 2. getCallHierarchy returns callers and callees
// 3. getDependencyGraph returns data flow dependencies
// 4. Scope respects nesting (inner scope sees outer, not vice versa)
//
// Will fail until the Context API is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"

// Forward declaration — ContextAPI
class ContextAPI {
public:
    struct Symbol {
        std::string name;
        std::string kind;  // "function", "variable", "parameter"
        std::string nodeId;
    };

    struct CallInfo {
        std::string functionId;
        std::string functionName;
        std::vector<std::string> callerIds;   // Functions that call this one
        std::vector<std::string> calleeIds;   // Functions called by this one
    };

    // Set the root AST
    void setRoot(ASTNode* root);

    // Get symbols in scope at a given node position
    std::vector<Symbol> getInScopeSymbols(const std::string& nodeId) const;

    // Get call hierarchy for a function
    CallInfo getCallHierarchy(const std::string& functionId) const;

    // Get data dependency node IDs for a given node
    std::vector<std::string> getDependencyGraph(const std::string& nodeId) const;
};

static bool hasSymbol(const std::vector<ContextAPI::Symbol>& symbols, const std::string& name) {
    return std::any_of(symbols.begin(), symbols.end(),
                       [&](const ContextAPI::Symbol& s) { return s.name == name; });
}

int main() {
    int passed = 0;
    int failed = 0;

    // Build test AST:
    // Module TestModule:
    //   Variable: globalVar
    //   Function add(x, y):
    //     Variable: localSum
    //     Return: localSum
    //   Function main():
    //     Variable: result
    //     ExpressionStatement: add(1, 2)  [FunctionCall]
    Module mod("m1", "TestModule", "python");

    Variable* globalVar = new Variable("gv1", "globalVar");
    mod.addChild("variables", globalVar);

    Function* addFn = new Function("f1", "add");
    Parameter* px = new Parameter("p1", "x");
    Parameter* py = new Parameter("p2", "y");
    addFn->addChild("parameters", px);
    addFn->addChild("parameters", py);

    Variable* localSum = new Variable("v1", "localSum");
    addFn->addChild("body", localSum);

    Return* ret = new Return();
    ret->id = "r1";
    VariableReference* retRef = new VariableReference("vr1", "localSum");
    ret->setChild("value", retRef);
    addFn->addChild("body", ret);

    mod.addChild("functions", addFn);

    Function* mainFn = new Function("f2", "main");
    Variable* result = new Variable("v2", "result");
    mainFn->addChild("body", result);

    ExpressionStatement* callStmt = new ExpressionStatement();
    callStmt->id = "es1";
    FunctionCall* call = new FunctionCall();
    call->id = "fc1";
    call->functionName = "add";
    IntegerLiteral* arg1 = new IntegerLiteral("i1", 1);
    IntegerLiteral* arg2 = new IntegerLiteral("i2", 2);
    call->addChild("arguments", arg1);
    call->addChild("arguments", arg2);
    callStmt->setChild("expression", call);
    mainFn->addChild("body", callStmt);

    mod.addChild("functions", mainFn);

    ContextAPI api;
    api.setRoot(&mod);

    // --- Test 1: Symbols in scope inside add function body ---
    {
        auto symbols = api.getInScopeSymbols("v1");  // localSum inside add()
        assert(hasSymbol(symbols, "x") && "Should see parameter x");
        assert(hasSymbol(symbols, "y") && "Should see parameter y");
        assert(hasSymbol(symbols, "localSum") && "Should see local variable");
        assert(hasSymbol(symbols, "globalVar") && "Should see module-level variable");
        assert(hasSymbol(symbols, "add") && "Should see sibling function");

        std::cout << "Test 1 PASS: Inner scope sees params, locals, globals, and functions" << std::endl;
        ++passed;
    }

    // --- Test 2: Module scope doesn't see function locals ---
    {
        auto symbols = api.getInScopeSymbols("m1");  // Module level
        assert(hasSymbol(symbols, "globalVar") && "Module should see its own variables");
        assert(hasSymbol(symbols, "add") && "Module should see its functions");
        assert(hasSymbol(symbols, "main") && "Module should see its functions");
        assert(!hasSymbol(symbols, "localSum") && "Module should NOT see function locals");
        assert(!hasSymbol(symbols, "x") && "Module should NOT see function parameters");

        std::cout << "Test 2 PASS: Module scope doesn't see function internals" << std::endl;
        ++passed;
    }

    // --- Test 3: main() scope sees its locals but not add() locals ---
    {
        auto symbols = api.getInScopeSymbols("v2");  // result inside main()
        assert(hasSymbol(symbols, "result") && "Should see own local");
        assert(hasSymbol(symbols, "globalVar") && "Should see module variable");
        assert(hasSymbol(symbols, "add") && "Should see sibling function");
        assert(!hasSymbol(symbols, "localSum") && "Should NOT see add()'s locals");
        assert(!hasSymbol(symbols, "x") && "Should NOT see add()'s parameters");

        std::cout << "Test 3 PASS: main() doesn't see add()'s locals/params" << std::endl;
        ++passed;
    }

    // --- Test 4: Call hierarchy for add() ---
    {
        auto callInfo = api.getCallHierarchy("f1");  // add function
        assert(callInfo.functionName == "add" && "Should be about 'add'");
        // main() calls add(), so add should have main as a caller
        assert(!callInfo.callerIds.empty() && "add() should have callers");
        bool mainCalls = std::any_of(callInfo.callerIds.begin(), callInfo.callerIds.end(),
                                      [](const std::string& id) { return id == "f2"; });
        assert(mainCalls && "main (f2) should be a caller of add");

        std::cout << "Test 4 PASS: Call hierarchy shows main() calls add()" << std::endl;
        ++passed;
    }

    // --- Test 5: Call hierarchy for main() ---
    {
        auto callInfo = api.getCallHierarchy("f2");  // main function
        assert(callInfo.functionName == "main" && "Should be about 'main'");
        // main() calls add()
        bool callsAdd = std::any_of(callInfo.calleeIds.begin(), callInfo.calleeIds.end(),
                                     [](const std::string& id) { return id == "f1"; });
        assert(callsAdd && "main should call add (f1)");

        std::cout << "Test 5 PASS: Call hierarchy shows main() calls add()" << std::endl;
        ++passed;
    }

    // --- Test 6: Dependency graph for return value ---
    {
        auto deps = api.getDependencyGraph("r1");  // Return node
        // The return depends on localSum (vr1 references it)
        assert(!deps.empty() && "Return should have dependencies");

        std::cout << "Test 6 PASS: Dependency graph returns dependencies" << std::endl;
        ++passed;
    }

    // Cleanup
    delete arg2;
    delete arg1;
    delete call;
    delete callStmt;
    delete result;
    delete mainFn;
    delete retRef;
    delete ret;
    delete localSum;
    delete py;
    delete px;
    delete addFn;
    delete globalVar;

    // --- Summary ---
    std::cout << "\n=== Step 62 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
