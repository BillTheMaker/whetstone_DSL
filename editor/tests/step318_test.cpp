// Step 318: Inference-to-Routing Bridge (12 tests)
// Connect annotation inference to routing decisions.
// inferRoutingAnnotations, estimateContextTokens, suggestWorkerType.

#include "HeadlessEditorState.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: build a function AST with given body nodes
static Function* makeFunction(const std::string& name,
                               const std::vector<ASTNode*>& bodyNodes = {}) {
    auto* fn = new Function();
    static int fid = 0;
    fn->id = "fn_" + std::to_string(++fid);
    fn->name = name;
    for (auto* n : bodyNodes) fn->addChild("body", n);
    return fn;
}

static FunctionCall* makeCall(const std::string& name) {
    auto* fc = new FunctionCall();
    static int cid = 0;
    fc->id = "fc_" + std::to_string(++cid);
    fc->functionName = name;
    return fc;
}

static ForLoop* makeLoop() {
    auto* lp = new ForLoop();
    static int lid = 0;
    lp->id = "lp_" + std::to_string(++lid);
    return lp;
}

// 1. Simple getter → @Automatability(deterministic) + @ContextWidth(local)
void test_simple_getter_routing() {
    TEST(simple_getter_routing);
    auto* fn = makeFunction("get_name");
    auto* ret = new Return();
    ret->id = "r1";
    fn->addChild("body", ret);
    auto* mod = new Module("root", "test", "python");
    mod->addChild("functions", fn);

    AnnotationInference inf;
    auto routing = inf.inferRoutingAnnotations(fn);
    bool hasDeterministic = false, hasLocal = false;
    for (const auto& r : routing) {
        if (r.annotationType == "AutomatabilityAnnotation" && r.value == "deterministic")
            hasDeterministic = true;
        if (r.annotationType == "ContextWidthAnnotation" && r.value == "local")
            hasLocal = true;
    }
    CHECK(hasDeterministic, "Simple getter → deterministic");
    CHECK(hasLocal, "Simple getter → local context");
    delete mod;
    PASS();
}

// 2. High complexity → @Ambiguity(high) + @Review(required)
void test_complex_function_routing() {
    TEST(complex_function_routing);
    // Create deeply nested loops for high cyclomatic complexity
    auto* inner = makeLoop();
    inner->addChild("body", makeCall("process"));
    auto* mid = makeLoop();
    mid->addChild("body", inner);
    auto* outer = makeLoop();
    outer->addChild("body", mid);

    auto* fn = makeFunction("complex_process");
    fn->addChild("body", outer);
    fn->addChild("body", makeCall("validate"));
    fn->addChild("body", makeCall("transform"));

    auto* mod = new Module("root", "test", "python");
    mod->addChild("functions", fn);

    AnnotationInference inf;
    auto routing = inf.inferRoutingAnnotations(fn);
    bool hasAmbiguity = false, hasReview = false;
    for (const auto& r : routing) {
        if (r.annotationType == "AmbiguityAnnotation") hasAmbiguity = true;
        if (r.annotationType == "ReviewAnnotation") hasReview = true;
    }
    CHECK(hasAmbiguity, "Complex → ambiguity annotation");
    CHECK(hasReview, "Complex → review annotation");
    delete mod;
    PASS();
}

// 3. Async + error handling → @Automatability(llm) + @ContextWidth(file)
void test_async_error_routing() {
    TEST(async_error_routing);
    auto* fn = new AsyncFunction();
    static int afid = 0;
    fn->id = "afn_" + std::to_string(++afid);
    fn->name = "fetch_data";
    fn->addChild("body", makeCall("try"));
    fn->addChild("body", makeCall("await"));

    auto* mod = new Module("root", "test", "python");
    mod->addChild("functions", fn);

    AnnotationInference inf;
    auto routing = inf.inferRoutingAnnotations(fn);
    bool hasLlm = false, hasFile = false;
    for (const auto& r : routing) {
        if (r.annotationType == "AutomatabilityAnnotation" && r.value == "llm")
            hasLlm = true;
        if (r.annotationType == "ContextWidthAnnotation" && r.value == "file")
            hasFile = true;
    }
    CHECK(hasLlm, "Async+error → llm automatability");
    CHECK(hasFile, "Async+error → file context");
    delete mod;
    PASS();
}

// 4. Single local transform → @Automatability(template) + @ContextWidth(local)
void test_simple_transform_routing() {
    TEST(simple_transform_routing);
    auto* fn = makeFunction("transform_data");
    fn->addChild("body", makeCall("map"));
    auto* ret = new Return();
    ret->id = "r2";
    fn->addChild("body", ret);

    auto* mod = new Module("root", "test", "python");
    mod->addChild("functions", fn);

    AnnotationInference inf;
    auto routing = inf.inferRoutingAnnotations(fn);
    bool hasTemplate = false, hasLocal = false;
    for (const auto& r : routing) {
        if (r.annotationType == "AutomatabilityAnnotation" && r.value == "template")
            hasTemplate = true;
        if (r.annotationType == "ContextWidthAnnotation" && r.value == "local")
            hasLocal = true;
    }
    CHECK(hasTemplate, "Simple transform → template");
    CHECK(hasLocal, "Simple transform → local");
    delete mod;
    PASS();
}

// 5. Cross-file calls → @ContextWidth(project)
void test_crossfile_routing() {
    TEST(crossfile_routing);
    auto* fn = makeFunction("orchestrate");
    // Calls that reference external modules
    fn->addChild("body", makeCall("auth.verify"));
    fn->addChild("body", makeCall("db.query"));
    fn->addChild("body", makeCall("cache.invalidate"));

    auto* mod = new Module("root", "test", "python");
    mod->addChild("functions", fn);

    AnnotationInference inf;
    auto routing = inf.inferRoutingAnnotations(fn);
    bool hasProject = false;
    for (const auto& r : routing) {
        if (r.annotationType == "ContextWidthAnnotation" && r.value == "project")
            hasProject = true;
    }
    CHECK(hasProject, "Cross-file calls → project context");
    delete mod;
    PASS();
}

// 6. estimateContextTokens — local should be smallest
void test_estimate_tokens_local() {
    TEST(estimate_tokens_local);
    auto* fn = makeFunction("small_fn");
    fn->addChild("body", makeCall("x"));

    auto* mod = new Module("root", "test", "python");
    mod->addChild("functions", fn);

    AnnotationInference inf;
    int localTokens = inf.estimateContextTokens(fn, "local");
    int fileTokens = inf.estimateContextTokens(fn, "file");
    int projectTokens = inf.estimateContextTokens(fn, "project");

    CHECK(localTokens > 0, "Local tokens > 0, got: " + std::to_string(localTokens));
    CHECK(fileTokens >= localTokens, "File >= local");
    CHECK(projectTokens >= fileTokens, "Project >= file");
    delete mod;
    PASS();
}

// 7. estimateContextTokens — larger AST gives more tokens
void test_estimate_tokens_scales() {
    TEST(estimate_tokens_scales);
    auto* fn1 = makeFunction("tiny");

    auto* fn2 = makeFunction("bigger");
    for (int i = 0; i < 10; i++)
        fn2->addChild("body", makeCall("step_" + std::to_string(i)));

    AnnotationInference inf;
    int small = inf.estimateContextTokens(fn1, "local");
    int big = inf.estimateContextTokens(fn2, "local");
    CHECK(big > small, "More nodes → more tokens");
    delete fn1;
    delete fn2;
    PASS();
}

// 8. suggestWorkerType — deterministic
void test_suggest_worker_deterministic() {
    TEST(suggest_worker_deterministic);
    AnnotationInference inf;
    std::vector<AnnotationInference::InferredAnnotation> annos = {
        {"n1", "AutomatabilityAnnotation", "strategy", "deterministic", "simple", 0.9},
        {"n1", "ContextWidthAnnotation", "width", "local", "local", 0.8}
    };
    std::string worker = inf.suggestWorkerType(annos);
    CHECK(worker == "deterministic", "Deterministic annotations → deterministic worker, got: " + worker);
    PASS();
}

// 9. suggestWorkerType — llm
void test_suggest_worker_llm() {
    TEST(suggest_worker_llm);
    AnnotationInference inf;
    std::vector<AnnotationInference::InferredAnnotation> annos = {
        {"n1", "AutomatabilityAnnotation", "strategy", "llm", "complex", 0.8},
        {"n1", "ContextWidthAnnotation", "width", "file", "cross-ref", 0.7}
    };
    std::string worker = inf.suggestWorkerType(annos);
    CHECK(worker == "llm", "LLM annotations → llm worker, got: " + worker);
    PASS();
}

// 10. suggestWorkerType — review required → human
void test_suggest_worker_human() {
    TEST(suggest_worker_human);
    AnnotationInference inf;
    std::vector<AnnotationInference::InferredAnnotation> annos = {
        {"n1", "ReviewAnnotation", "required", "true", "complex", 0.9},
        {"n1", "AmbiguityAnnotation", "level", "high", "ambiguous", 0.85}
    };
    std::string worker = inf.suggestWorkerType(annos);
    CHECK(worker == "human", "Review+ambiguity → human worker, got: " + worker);
    PASS();
}

// 11. Manual annotations override inferred routing
void test_manual_override() {
    TEST(manual_override);
    auto* fn = makeFunction("simple_fn");
    fn->addChild("body", makeCall("x"));

    // Attach a manual @Automatability(llm) annotation
    auto* manual = new AutomatabilityAnnotation();
    manual->id = "manual_aa";
    manual->strategy = "llm";
    fn->addChild("annotations", manual);

    auto* mod = new Module("root", "test", "python");
    mod->addChild("functions", fn);

    AnnotationInference inf;
    auto routing = inf.inferRoutingAnnotations(fn);
    // Should not infer Automatability since one already exists
    bool hasAutoInferred = false;
    for (const auto& r : routing) {
        if (r.annotationType == "AutomatabilityAnnotation")
            hasAutoInferred = true;
    }
    CHECK(!hasAutoInferred, "Manual annotation prevents inferred automatability");
    delete mod;
    PASS();
}

// 12. Combined inference + routing pipeline
void test_combined_pipeline() {
    TEST(combined_pipeline);
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.openBuffer("test",
        "def get_value():\n    return 42\n\n"
        "def complex_process():\n"
        "    for i in range(n):\n"
        "        for j in range(m):\n"
        "            data = transform(i, j)\n"
        "    return data\n",
        "python");

    auto* ast = state.activeAST();
    CHECK(ast != nullptr, "Has AST");

    AnnotationInference inf;
    // Full inference
    auto all = inf.inferAll(ast);
    CHECK(!all.empty(), "inferAll returns suggestions");

    // Routing inference on each function
    auto fns = ast->getChildren("functions");
    int routingCount = 0;
    for (auto* fn : fns) {
        auto routing = inf.inferRoutingAnnotations(fn);
        routingCount += (int)routing.size();
    }
    CHECK(routingCount > 0, "Routing annotations inferred for functions");

    // Worker suggestion from routing
    auto firstRouting = inf.inferRoutingAnnotations(fns.empty() ? ast : fns[0]);
    if (!firstRouting.empty()) {
        std::string worker = inf.suggestWorkerType(firstRouting);
        CHECK(!worker.empty(), "Worker suggestion non-empty");
        CHECK(worker == "deterministic" || worker == "template" ||
              worker == "slm" || worker == "llm" || worker == "human",
              "Worker is valid type: " + worker);
    }
    PASS();
}

int main() {
    std::cout << "Step 318: Inference-to-Routing Bridge\n";
    try {
    test_simple_getter_routing();
    test_complex_function_routing();
    test_async_error_routing();
    test_simple_transform_routing();
    test_crossfile_routing();
    test_estimate_tokens_local();
    test_estimate_tokens_scales();
    test_suggest_worker_deterministic();
    test_suggest_worker_llm();
    test_suggest_worker_human();
    test_manual_override();
    test_combined_pipeline();
    } catch (const std::exception& e) {
        std::cout << "\nFATAL: " << e.what() << "\n";
        return 1;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
