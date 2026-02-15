// Step 314: Generalized Annotation Inference Engine (12 tests)
// Verifies AnnotationInference::inferAll covers all 8 subjects:
// memory delegation, async/exec, pure, tail-call, visibility,
// exception, blocking, parallel, complexity, and loop inference.

#include "AnnotationInference.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"
#include "ast/ClassDeclaration.h"
#include "ast/AsyncNodes.h"
#include "ast/HostBoundary.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <set>
#include <algorithm>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: build a simple function with body statements
static std::unique_ptr<Module> makeModule(const std::string& lang = "python") {
    return std::make_unique<Module>("mod1", "testmod", lang);
}

static Function* addFunction(Module* mod, const std::string& name,
                             const std::string& id = "") {
    auto fn = std::make_unique<Function>(id.empty() ? "fn_" + name : id, name);
    auto* ptr = fn.get();
    mod->addChild("functions", fn.release());
    return ptr;
}

// 1. Memory delegation — inferAll delegates to MemoryStrategyInference
void test_memory_delegation() {
    TEST(memory_delegation);
    auto mod = makeModule("python");
    auto* fn = addFunction(mod.get(), "process");
    auto var = std::make_unique<Variable>("v1", "data");
    fn->addChild("body", var.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    // Should have memory suggestions (from MemoryStrategyInference) plus other inferences
    bool hasMemory = false;
    for (const auto& r : results) {
        if (r.annotationType == "ReclaimAnnotation" || r.annotationType == "OwnerAnnotation") {
            hasMemory = true;
            break;
        }
    }
    CHECK(hasMemory, "Expected memory annotations from delegation");
    PASS();
}

// 2. Async function → ExecAnnotation(async)
void test_async_exec_inference() {
    TEST(async_exec_inference);
    auto mod = makeModule("python");
    auto fn = std::make_unique<AsyncFunction>("af1", "fetch_data");
    mod->addChild("functions", fn.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "ExecAnnotation" && r.value == "async") {
            found = true;
            CHECK(r.confidence >= 0.9, "Async confidence should be >= 0.9");
            CHECK(r.nodeId == "af1", "Should target async function node");
        }
    }
    CHECK(found, "Expected ExecAnnotation for async function");
    PASS();
}

// 3. Pure function detection (no side effects)
void test_pure_function_inference() {
    TEST(pure_function_inference);
    auto mod = makeModule("python");
    auto* fn = addFunction(mod.get(), "add");
    // Body: just return x + y (no side effects)
    auto ret = std::make_unique<Return>();
    ret->id = "r1";
    auto binop = std::make_unique<BinaryOperation>("bo1", "+");
    auto lhs = std::make_unique<VariableReference>("vr1", "x");
    auto rhs = std::make_unique<VariableReference>("vr2", "y");
    binop->addChild("left", lhs.release());
    binop->addChild("right", rhs.release());
    ret->addChild("value", binop.release());
    fn->addChild("body", ret.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "PureAnnotation" && r.nodeId == fn->id) {
            found = true;
            CHECK(r.confidence >= 0.5, "Pure confidence should be reasonable");
        }
    }
    CHECK(found, "Expected PureAnnotation for side-effect-free function");
    PASS();
}

// 4. Tail-recursive function detection
void test_tail_call_inference() {
    TEST(tail_call_inference);
    auto mod = makeModule("python");
    auto* fn = addFunction(mod.get(), "factorial");
    // Body: return factorial(n-1)
    auto ret = std::make_unique<Return>();
    ret->id = "r1";
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "factorial";
    ret->addChild("value", call.release());
    fn->addChild("body", ret.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "TailCallAnnotation" && r.nodeId == fn->id) {
            found = true;
            CHECK(r.confidence >= 0.7, "Tail call confidence should be >= 0.7");
        }
    }
    CHECK(found, "Expected TailCallAnnotation for tail-recursive function");
    PASS();
}

// 5. Method → VisibilityAnnotation
void test_visibility_inference() {
    TEST(visibility_inference);
    auto mod = makeModule("python");
    auto method = std::make_unique<MethodDeclaration>("md1", "get_name");
    method->className = "User";
    mod->addChild("functions", method.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "VisibilityAnnotation" && r.nodeId == "md1") {
            found = true;
            CHECK(r.value == "public", "Default visibility should be public");
        }
    }
    CHECK(found, "Expected VisibilityAnnotation for method");
    PASS();
}

// 6. Exception handling detection
void test_exception_inference() {
    TEST(exception_inference);
    auto mod = makeModule("python");
    auto* fn = addFunction(mod.get(), "risky_op");
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "try";
    fn->addChild("body", call.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "ExceptionAnnotation" && r.nodeId == fn->id) {
            found = true;
            CHECK(r.value == "unchecked", "Exception style should be unchecked");
        }
    }
    CHECK(found, "Expected ExceptionAnnotation for try/catch function");
    PASS();
}

// 7. Blocking call detection
void test_blocking_inference() {
    TEST(blocking_inference);
    auto mod = makeModule("python");
    auto* fn = addFunction(mod.get(), "read_file");
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "read";
    fn->addChild("body", call.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "BlockingAnnotation" && r.nodeId == fn->id) {
            found = true;
            CHECK(r.value == "io", "Blocking kind should be io");
        }
    }
    CHECK(found, "Expected BlockingAnnotation for IO function");
    PASS();
}

// 8. Parallel pattern detection (ScheduleTask node)
void test_parallel_inference() {
    TEST(parallel_inference);
    auto mod = makeModule("go");
    auto* fn = addFunction(mod.get(), "dispatch");
    auto task = std::make_unique<ScheduleTask>();
    task->id = "st1";
    fn->addChild("body", task.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "ParallelAnnotation" && r.nodeId == fn->id) {
            found = true;
            CHECK(r.value == "task", "Parallel kind should be task");
        }
    }
    CHECK(found, "Expected ParallelAnnotation for parallel dispatch");
    PASS();
}

// 9. Complexity inference (nested loops → O(n^2))
void test_complexity_inference() {
    TEST(complexity_inference);
    auto mod = makeModule("python");
    auto* fn = addFunction(mod.get(), "matrix_multiply");
    // Nested loops: for → for
    auto outer = std::make_unique<ForLoop>();
    outer->id = "fl1";
    auto inner = std::make_unique<ForLoop>();
    inner->id = "fl2";
    outer->addChild("body", inner.release());
    fn->addChild("body", outer.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "ComplexityAnnotation" && r.nodeId == fn->id) {
            found = true;
            CHECK(r.value == "O(n^2)", "Nested loops should infer O(n^2), got: " + r.value);
        }
    }
    CHECK(found, "Expected ComplexityAnnotation for nested loop function");
    PASS();
}

// 10. Loop annotation inference
void test_loop_inference() {
    TEST(loop_inference);
    auto mod = makeModule("python");
    auto* fn = addFunction(mod.get(), "process");
    auto loop = std::make_unique<ForLoop>();
    loop->id = "fl1";
    fn->addChild("body", loop.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    bool found = false;
    for (const auto& r : results) {
        if (r.annotationType == "LoopAnnotation" && r.nodeId == "fl1") {
            found = true;
            CHECK(r.value == "vectorize", "Loop hint should be vectorize");
        }
    }
    CHECK(found, "Expected LoopAnnotation for loop node");
    PASS();
}

// 11. Existing annotations are not duplicated
void test_skip_existing_annotations() {
    TEST(skip_existing_annotations);
    auto mod = makeModule("python");
    auto fn = std::make_unique<AsyncFunction>("af1", "already_annotated");
    // Pre-annotate with ExecAnnotation
    auto exec = std::make_unique<ExecAnnotation>();
    exec->id = "ea1";
    exec->mode = "async";
    fn->addChild("annotations", exec.release());
    mod->addChild("functions", fn.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    int execCount = 0;
    for (const auto& r : results) {
        if (r.annotationType == "ExecAnnotation" && r.nodeId == "af1") {
            execCount++;
        }
    }
    CHECK(execCount == 0, "Should skip ExecAnnotation when already present, got: " + std::to_string(execCount));
    PASS();
}

// 12. Confidence scores are in valid range
void test_confidence_range() {
    TEST(confidence_range);
    auto mod = makeModule("python");
    // Build a function with many patterns
    auto fn = std::make_unique<AsyncFunction>("af1", "complex");
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "read";
    fn->addChild("body", call.release());
    auto loop = std::make_unique<ForLoop>();
    loop->id = "fl1";
    fn->addChild("body", loop.release());
    mod->addChild("functions", fn.release());

    AnnotationInference inf;
    auto results = inf.inferAll(mod.get());

    CHECK(!results.empty(), "Should produce some inferences");
    for (const auto& r : results) {
        CHECK(r.confidence >= 0.0 && r.confidence <= 1.0,
              "Confidence must be in [0, 1], got: " + std::to_string(r.confidence) +
              " for " + r.annotationType);
        CHECK(!r.nodeId.empty(), "nodeId must not be empty for " + r.annotationType);
        CHECK(!r.annotationType.empty(), "annotationType must not be empty");
    }
    PASS();
}

int main() {
    std::cout << "Step 314: Generalized Annotation Inference Engine\n";
    test_memory_delegation();
    test_async_exec_inference();
    test_pure_function_inference();
    test_tail_call_inference();
    test_visibility_inference();
    test_exception_inference();
    test_blocking_inference();
    test_parallel_inference();
    test_complexity_inference();
    test_loop_inference();
    test_skip_existing_annotations();
    test_confidence_range();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
