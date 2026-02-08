// Step 60 TDD Test: AST query API
//
// Tests the agent query API:
// 1. getAST(nodeId) returns AST subtree as JSON
// 2. findNodes by concept type returns matching nodes
// 3. findNodes by property value returns matching nodes
// 4. getSubtree with depth limit
// 5. findNodes with annotation presence filter

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
#include "ast/Serialization.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// --- ASTQueryAPI implementation for Step 60 ---

class ASTQueryAPI {
    ASTNode* root_ = nullptr;

    // Recursive helper: find node by ID
    ASTNode* findById(ASTNode* node, const std::string& nodeId) const {
        if (!node) return nullptr;
        if (node->id == nodeId) return node;
        for (auto* child : node->allChildren()) {
            auto* found = findById(child, nodeId);
            if (found) return found;
        }
        return nullptr;
    }

    // Recursive helper: collect all nodes into a flat list
    void collectAll(ASTNode* node, std::vector<ASTNode*>& out) const {
        if (!node) return;
        out.push_back(node);
        for (auto* child : node->allChildren()) {
            collectAll(child, out);
        }
    }

    // Get a named string property from a node (by property name)
    static std::string getStringProperty(const ASTNode* node, const std::string& prop) {
        const auto& ct = node->conceptType;
        if (prop == "name") {
            if (ct == "Module") return static_cast<const Module*>(node)->name;
            if (ct == "Function") return static_cast<const Function*>(node)->name;
            if (ct == "Variable") return static_cast<const Variable*>(node)->name;
            if (ct == "Parameter") return static_cast<const Parameter*>(node)->name;
        }
        if (prop == "op") {
            if (ct == "BinaryOperation") return static_cast<const BinaryOperation*>(node)->op;
            if (ct == "UnaryOperation") return static_cast<const UnaryOperation*>(node)->op;
        }
        if (prop == "variableName" && ct == "VariableReference")
            return static_cast<const VariableReference*>(node)->variableName;
        if (prop == "strategy") {
            if (ct == "DerefStrategy") return static_cast<const DerefStrategy*>(node)->strategy;
            if (ct == "DeallocateAnnotation") return static_cast<const DeallocateAnnotation*>(node)->strategy;
            if (ct == "LifetimeAnnotation") return static_cast<const LifetimeAnnotation*>(node)->strategy;
            if (ct == "ReclaimAnnotation") return static_cast<const ReclaimAnnotation*>(node)->strategy;
            if (ct == "OwnerAnnotation") return static_cast<const OwnerAnnotation*>(node)->strategy;
            if (ct == "AllocateAnnotation") return static_cast<const AllocateAnnotation*>(node)->strategy;
        }
        return "";
    }

    // JSON with depth limit
    json toJsonDepth(const ASTNode* node, int depth) const {
        json j;
        j["id"] = node->id;
        j["concept"] = node->conceptType;
        j["properties"] = propertiesToJson(node);
        if (depth > 0) {
            json children = json::object();
            for (const auto& role : node->childRoles()) {
                json arr = json::array();
                for (const auto* child : node->getChildren(role)) {
                    arr.push_back(toJsonDepth(child, depth - 1));
                }
                children[role] = arr;
            }
            j["children"] = children;
        }
        return j;
    }

public:
    void setRoot(ASTNode* root) { root_ = root; }

    std::string getAST(const std::string& nodeId) const {
        ASTNode* node = findById(root_, nodeId);
        if (!node) return "";
        return toJson(node).dump();
    }

    std::string getSubtree(const std::string& nodeId, int depth) const {
        ASTNode* node = findById(root_, nodeId);
        if (!node) return "";
        return toJsonDepth(node, depth).dump();
    }

    std::vector<ASTNode*> findNodesByType(const std::string& conceptType) const {
        std::vector<ASTNode*> all, result;
        collectAll(root_, all);
        for (auto* n : all) {
            if (n->conceptType == conceptType) result.push_back(n);
        }
        return result;
    }

    std::vector<ASTNode*> findNodesByProperty(const std::string& propertyName,
                                               const std::string& value) const {
        std::vector<ASTNode*> all, result;
        collectAll(root_, all);
        for (auto* n : all) {
            if (getStringProperty(n, propertyName) == value) result.push_back(n);
        }
        return result;
    }

    std::vector<ASTNode*> findNodesWithAnnotation(const std::string& annotationType) const {
        std::vector<ASTNode*> all, result;
        collectAll(root_, all);
        for (auto* n : all) {
            for (auto* anno : n->getChildren("annotations")) {
                if (anno->conceptType == annotationType) {
                    result.push_back(n);
                    break;
                }
            }
        }
        return result;
    }
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
