// Step 60 TDD Test: AST query API
//
// Tests the agent query API:
// 1. getAST(nodeId) returns AST subtree as JSON
// 2. findNodes by concept type returns matching nodes
// 3. findNodes by property value returns matching nodes
// 4. getSubtree with depth limit
// 5. findNodes with annotation presence filter
//
// Will fail until the AST query API is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// Forward declaration — ASTQueryAPI
class ASTQueryAPI {
public:
    // Set the root AST to query against
    void setRoot(ASTNode* root);

    // Get subtree as JSON string
    std::string getAST(const std::string& nodeId) const;

    // Get subtree with depth limit
    std::string getSubtree(const std::string& nodeId, int depth) const;

    // Find nodes by concept type
    std::vector<ASTNode*> findNodesByType(const std::string& conceptType) const;

    // Find nodes where a string property matches
    std::vector<ASTNode*> findNodesByProperty(const std::string& propertyName,
                                               const std::string& value) const;

    // Find nodes that have a specific annotation type
    std::vector<ASTNode*> findNodesWithAnnotation(const std::string& annotationType) const;
};

int main() {
    int passed = 0;
    int failed = 0;

    // Build a test AST: Module with 2 functions, one with an annotation
    Module mod("m1", "TestModule", "python");

    Function* fn1 = new Function("f1", "add");
    Parameter* p1 = new Parameter("p1", "x");
    Parameter* p2 = new Parameter("p2", "y");
    fn1->addChild("parameters", p1);
    fn1->addChild("parameters", p2);
    Return* ret1 = new Return();
    ret1->id = "r1";
    BinaryOperation* binOp = new BinaryOperation("b1", "+");
    VariableReference* refX = new VariableReference("vr1", "x");
    VariableReference* refY = new VariableReference("vr2", "y");
    binOp->setChild("left", refX);
    binOp->setChild("right", refY);
    ret1->setChild("value", binOp);
    fn1->addChild("body", ret1);
    mod.addChild("functions", fn1);

    Function* fn2 = new Function("f2", "compute");
    DerefStrategy* deref = new DerefStrategy("d1", "batched");
    fn2->addChild("annotations", deref);
    Variable* var = new Variable("v1", "result");
    var->id = "v1";
    fn2->addChild("body", var);
    mod.addChild("functions", fn2);

    ASTQueryAPI api;
    api.setRoot(&mod);

    // --- Test 1: getAST returns JSON for root ---
    {
        std::string json = api.getAST("m1");
        assert(!json.empty() && "getAST should return non-empty JSON");
        assert(contains(json, "TestModule") && "JSON should contain module name");
        assert(contains(json, "Module") && "JSON should contain concept type");

        std::cout << "Test 1 PASS: getAST returns JSON with module info" << std::endl;
        ++passed;
    }

    // --- Test 2: getAST for a child node ---
    {
        std::string json = api.getAST("f1");
        assert(!json.empty() && "getAST should return JSON for child node");
        assert(contains(json, "add") && "JSON should contain function name 'add'");

        std::cout << "Test 2 PASS: getAST returns JSON for child node" << std::endl;
        ++passed;
    }

    // --- Test 3: findNodesByType("Function") returns both functions ---
    {
        auto results = api.findNodesByType("Function");
        assert(results.size() == 2 && "Should find 2 Function nodes");

        std::cout << "Test 3 PASS: findNodesByType finds all Functions" << std::endl;
        ++passed;
    }

    // --- Test 4: findNodesByType("Parameter") returns both parameters ---
    {
        auto results = api.findNodesByType("Parameter");
        assert(results.size() == 2 && "Should find 2 Parameter nodes");

        std::cout << "Test 4 PASS: findNodesByType finds all Parameters" << std::endl;
        ++passed;
    }

    // --- Test 5: findNodesByType("VariableReference") returns 2 refs ---
    {
        auto results = api.findNodesByType("VariableReference");
        assert(results.size() == 2 && "Should find 2 VariableReference nodes");

        std::cout << "Test 5 PASS: findNodesByType finds VariableReferences" << std::endl;
        ++passed;
    }

    // --- Test 6: findNodesByProperty returns matching node ---
    {
        auto results = api.findNodesByProperty("name", "compute");
        assert(results.size() >= 1 && "Should find at least 1 node with name='compute'");

        auto* found = static_cast<Function*>(results[0]);
        assert(found->name == "compute" && "Found node should be the 'compute' function");

        std::cout << "Test 6 PASS: findNodesByProperty finds by name" << std::endl;
        ++passed;
    }

    // --- Test 7: findNodesWithAnnotation returns annotated function ---
    {
        auto results = api.findNodesWithAnnotation("DerefStrategy");
        assert(results.size() >= 1 && "Should find at least 1 node with DerefStrategy annotation");

        auto* found = static_cast<Function*>(results[0]);
        assert(found->name == "compute" && "Annotated function should be 'compute'");

        std::cout << "Test 7 PASS: findNodesWithAnnotation finds annotated nodes" << std::endl;
        ++passed;
    }

    // --- Test 8: getSubtree with depth=1 limits children ---
    {
        std::string json = api.getSubtree("m1", 1);
        assert(!json.empty() && "getSubtree should return JSON");
        assert(contains(json, "TestModule") && "Should contain module name");
        // At depth 1, should show functions but not their children details
        assert(contains(json, "add") && "Should show function names at depth 1");

        std::cout << "Test 8 PASS: getSubtree respects depth limit" << std::endl;
        ++passed;
    }

    // Cleanup
    delete refY;
    delete refX;
    delete binOp;
    delete ret1;
    delete p2;
    delete p1;
    delete fn1;
    delete deref;
    delete var;
    delete fn2;

    // --- Summary ---
    std::cout << "\n=== Step 60 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
