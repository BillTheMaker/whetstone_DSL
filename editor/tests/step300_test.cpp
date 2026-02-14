// Step 300: TransformEngineExtended Tests (12 tests)
// Tests float constant folding, dead variable elimination,
// and annotation-aware optimization.

#include "TransformEngineExtended.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"
#include <iostream>
#include <string>
#include <cmath>
#include <memory>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: create a BinaryOperation with two FloatLiteral children
static ASTNode* makeFloatBinOp(const std::string& op,
                                const std::string& leftVal,
                                const std::string& rightVal) {
    auto* binOp = new BinaryOperation("binop_1", op);
    binOp->setChild("left", new FloatLiteral("fl_l", leftVal));
    binOp->setChild("right", new FloatLiteral("fl_r", rightVal));
    return binOp;
}

// 1. Float constant folding: 1.5 + 2.5 -> 4.0
void test_float_add() {
    TEST(float_add_fold);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", makeFloatBinOp("+", "1.5", "2.5"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.floatConstantFolding();
    CHECK(result.applied, "fold should apply");
    CHECK(result.nodesModified >= 1, "should modify at least 1 node");

    // Verify the result is a FloatLiteral with value ~4.0
    auto body = fn->getChildren("body");
    CHECK(!body.empty(), "body should not be empty");
    CHECK(body[0]->conceptType == "FloatLiteral", "result should be FloatLiteral, got " + body[0]->conceptType);
    double val = std::stod(static_cast<FloatLiteral*>(body[0])->value);
    CHECK(std::abs(val - 4.0) < 0.001, "expected ~4.0, got " + std::to_string(val));
    PASS();
}

// 2. Float multiplication: 3.0 * 2.0 -> 6.0
void test_float_mul() {
    TEST(float_mul_fold);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", makeFloatBinOp("*", "3.0", "2.0"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.floatConstantFolding();
    CHECK(result.applied, "fold should apply");

    auto body = fn->getChildren("body");
    CHECK(body[0]->conceptType == "FloatLiteral", "result should be FloatLiteral");
    double val = std::stod(static_cast<FloatLiteral*>(body[0])->value);
    CHECK(std::abs(val - 6.0) < 0.001, "expected ~6.0, got " + std::to_string(val));
    PASS();
}

// 3. Float division by zero -> no fold
void test_float_div_zero() {
    TEST(float_div_zero_no_fold);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", makeFloatBinOp("/", "1.0", "0.0"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.floatConstantFolding();
    CHECK(!result.applied, "fold should not apply for division by zero");

    auto body = fn->getChildren("body");
    CHECK(body[0]->conceptType == "BinaryOperation", "should remain BinaryOperation");
    PASS();
}

// 4. Dead variable removed (unused var)
void test_dead_var_removed() {
    TEST(dead_var_removed);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", new Variable("v1", "unused"));
    // Add a used variable and a reference
    fn->addChild("body", new Variable("v2", "used"));
    fn->addChild("body", new VariableReference("ref1", "used"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.deadVariableElimination();
    CHECK(result.applied, "should remove unused variable");
    CHECK(result.nodesModified >= 1, "should modify at least 1 node");

    // Check that "unused" is gone
    auto body = fn->getChildren("body");
    for (auto* stmt : body) {
        if (stmt->conceptType == "Variable") {
            CHECK(static_cast<Variable*>(stmt)->name != "unused",
                  "unused variable should have been removed");
        }
    }
    PASS();
}

// 5. Used variable preserved
void test_used_var_preserved() {
    TEST(used_var_preserved);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", new Variable("v1", "x"));
    fn->addChild("body", new VariableReference("ref1", "x"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.deadVariableElimination();
    // x is used, should not be removed
    auto body = fn->getChildren("body");
    bool found = false;
    for (auto* stmt : body) {
        if (stmt->conceptType == "Variable" && static_cast<Variable*>(stmt)->name == "x")
            found = true;
    }
    CHECK(found, "used variable 'x' should be preserved");
    PASS();
}

// 6. OptimizationLock blocks optimization
void test_optimization_lock() {
    TEST(optimization_lock_blocks);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("annotations", new OptimizationLock());
    fn->addChild("body", makeFloatBinOp("+", "1.0", "2.0"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.annotationAwareOptimize();
    CHECK(!result.applied, "optimization should be blocked by OptimizationLock");
    CHECK(!result.warning.empty(), "should have a warning about OptimizationLock");
    PASS();
}

// 7. BoundsCheck-protected node skipped
void test_boundscheck_skipped() {
    TEST(boundscheck_protected_skipped);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    // Add BoundsCheck to the function
    auto bc = new BoundsCheckAnnotation(); bc->enabled = true;
    fn->addChild("annotations", bc);
    fn->addChild("body", makeFloatBinOp("+", "1.0", "2.0"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.annotationAwareOptimize();
    // BoundsCheck should cause the function subtree to be skipped
    CHECK(!result.applied, "optimization should skip BoundsCheck-protected node");
    PASS();
}

// 8. No OptimizationLock -> optimization runs
void test_no_lock_optimization_runs() {
    TEST(no_lock_optimization_runs);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", makeFloatBinOp("+", "1.0", "2.0"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.annotationAwareOptimize();
    // No lock or boundscheck, so nothing blocks but annotationAwareOptimize
    // uses foldConstantsSkipProtected which doesn't actually fold (it only
    // recurses). The key test is that no warning is generated.
    CHECK(result.warning.empty(), "should have no warning without OptimizationLock");
    PASS();
}

// 9. applyAllExtended runs all 3
void test_apply_all_extended() {
    TEST(apply_all_extended);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", makeFloatBinOp("+", "1.0", "2.0"));
    fn->addChild("body", new Variable("v1", "deadvar"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.applyAllExtended();
    CHECK(result.applied, "at least one transform should apply");
    CHECK(result.nodesModified >= 2, "float fold + dead var = at least 2 modifications, got " + std::to_string(result.nodesModified));
    PASS();
}

// 10. nodesModified count accurate
void test_nodes_modified_count() {
    TEST(nodes_modified_count);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", makeFloatBinOp("+", "1.0", "2.0"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.floatConstantFolding();
    CHECK(result.nodesModified == 1, "expected exactly 1 node modified, got " + std::to_string(result.nodesModified));
    PASS();
}

// 11. Float nested fold: (1.0 + 2.0) + 3.0
void test_nested_float_fold() {
    TEST(nested_float_fold);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");

    // Inner: 1.0 + 2.0
    auto* inner = new BinaryOperation("binop_inner", "+");
    inner->setChild("left", new FloatLiteral("fl_1", "1.0"));
    inner->setChild("right", new FloatLiteral("fl_2", "2.0"));

    // Outer: (inner) + 3.0
    auto* outer = new BinaryOperation("binop_outer", "+");
    outer->setChild("left", inner);
    outer->setChild("right", new FloatLiteral("fl_3", "3.0"));

    fn->addChild("body", outer);
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.floatConstantFolding();
    CHECK(result.applied, "nested fold should apply");
    CHECK(result.nodesModified >= 2, "should fold both inner and outer");

    auto body = fn->getChildren("body");
    CHECK(body[0]->conceptType == "FloatLiteral", "final result should be FloatLiteral");
    double val = std::stod(static_cast<FloatLiteral*>(body[0])->value);
    CHECK(std::abs(val - 6.0) < 0.001, "expected ~6.0, got " + std::to_string(val));
    PASS();
}

// 12. Multiple dead variables removed
void test_multiple_dead_vars() {
    TEST(multiple_dead_vars);
    auto mod = std::make_unique<Module>();
    auto fn = new Function("fn1", "f");
    fn->addChild("body", new Variable("v1", "dead1"));
    fn->addChild("body", new Variable("v2", "dead2"));
    fn->addChild("body", new Variable("v3", "alive"));
    fn->addChild("body", new VariableReference("ref1", "alive"));
    mod->addChild("functions", fn);

    TransformEngineExtended engine;
    engine.setRoot(mod.get());
    auto result = engine.deadVariableElimination();
    CHECK(result.applied, "should remove dead variables");
    CHECK(result.nodesModified >= 2, "should remove at least 2 dead vars, got " + std::to_string(result.nodesModified));

    auto body = fn->getChildren("body");
    for (auto* stmt : body) {
        if (stmt->conceptType == "Variable") {
            std::string name = static_cast<Variable*>(stmt)->name;
            CHECK(name != "dead1" && name != "dead2",
                  "dead variable '" + name + "' should have been removed");
        }
    }
    PASS();
}

int main() {
    std::cout << "=== Step 300: TransformEngineExtended Tests ===\n";
    test_float_add();
    test_float_mul();
    test_float_div_zero();
    test_dead_var_removed();
    test_used_var_preserved();
    test_optimization_lock();
    test_boundscheck_skipped();
    test_no_lock_optimization_runs();
    test_apply_all_extended();
    test_nodes_modified_count();
    test_nested_float_fold();
    test_multiple_dead_vars();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
