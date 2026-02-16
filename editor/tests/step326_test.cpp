// Step 326: RoutingEngine — Annotation-to-Dispatch Logic (12 tests)
// Tests routing rules: explicit override, ambiguity, context width,
// pattern defaults, batch routing, confidence, context budget.

#include "RoutingEngine.h"
#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& name,
                          const std::string& workerType = "",
                          const std::string& contextWidth = "local",
                          bool reviewRequired = false) {
    WorkItem wi;
    wi.id = "wi-" + name;
    wi.nodeId = "n-" + name;
    wi.nodeName = name;
    wi.nodeType = "Function";
    wi.bufferId = "buf";
    wi.workerType = workerType;
    wi.contextWidth = contextWidth;
    wi.reviewRequired = reviewRequired;
    wi.priority = "medium";
    wi.createdAt = workItemTimestamp();
    return wi;
}

RoutingEngine engine;

// 1. Explicit @Automatability overrides inferred routing
void test_explicit_override() {
    TEST(explicit_override);
    auto wi = makeItem("compute", "deterministic", "project");
    auto d = engine.route(wi);
    CHECK(d.workerType == "deterministic", "explicit deterministic honored");
    CHECK(d.confidence > 0.9f, "high confidence for explicit");
    CHECK(d.reasoning.find("Explicit") != std::string::npos, "reasoning mentions explicit");
    PASS();
}

// 2. High ambiguity → human
void test_ambiguity_human() {
    TEST(ambiguity_human);
    auto wi = makeItem("ambiguous", "human", "local");
    auto d = engine.route(wi);
    CHECK(d.workerType == "human", "routed to human");
    CHECK(d.reviewRequired, "review required for human");
    PASS();
}

// 3. Low complexity + local → deterministic (getter pattern)
void test_getter_deterministic() {
    TEST(getter_deterministic);
    auto wi = makeItem("getName", "", "local");
    auto d = engine.route(wi);
    CHECK(d.workerType == "deterministic", "getter → deterministic");
    CHECK(d.agentRole == "refactor", "agent role = refactor");
    PASS();
}

// 4. Setter pattern → deterministic
void test_setter_deterministic() {
    TEST(setter_deterministic);
    auto wi = makeItem("setValue", "", "local");
    auto d = engine.route(wi);
    CHECK(d.workerType == "deterministic", "setter → deterministic");
    PASS();
}

// 5. Cross-project → LLM
void test_cross_project_llm() {
    TEST(cross_project_llm);
    auto wi = makeItem("orchestrate", "", "cross-project");
    auto d = engine.route(wi);
    CHECK(d.workerType == "llm", "cross-project → llm");
    CHECK(d.contextBudgetTokens == 16000, "16000 token budget");
    PASS();
}

// 6. Project context → LLM
void test_project_llm() {
    TEST(project_llm);
    auto wi = makeItem("buildGraph", "", "project");
    auto d = engine.route(wi);
    CHECK(d.workerType == "llm", "project → llm");
    CHECK(d.contextBudgetTokens == 8000, "8000 token budget");
    PASS();
}

// 7. Review annotation sets reviewRequired
void test_review_annotation() {
    TEST(review_annotation);
    auto wi = makeItem("sensitive", "", "local", true);
    auto d = engine.route(wi);
    CHECK(d.reviewRequired, "review preserved from annotation");
    // Should also boost confidence slightly
    CHECK(d.confidence > 0.5f, "confidence boosted by annotation signal");
    PASS();
}

// 8. Batch routing
void test_batch_routing() {
    TEST(batch_routing);
    std::vector<WorkItem> items = {
        makeItem("getName", "", "local"),
        makeItem("compute", "", "project"),
        makeItem("transform", "human", "local"),
    };
    auto decisions = engine.routeBatch(items);
    CHECK(decisions.size() == 3, "3 decisions");
    CHECK(decisions[0].workerType == "deterministic", "first=deterministic");
    CHECK(decisions[1].workerType == "llm", "second=llm");
    CHECK(decisions[2].workerType == "human", "third=human");
    PASS();
}

// 9. No-annotation defaults (local context, no patterns)
void test_default_local() {
    TEST(default_local);
    auto wi = makeItem("processData", "", "local");
    auto d = engine.route(wi);
    CHECK(d.workerType == "slm", "local default → slm");
    CHECK(d.contextBudgetTokens == 500, "500 token budget");
    PASS();
}

// 10. File context default
void test_default_file() {
    TEST(default_file);
    auto wi = makeItem("handleRequest", "", "file");
    auto d = engine.route(wi);
    CHECK(d.workerType == "slm", "file default → slm");
    CHECK(d.contextBudgetTokens == 2000, "2000 token budget");
    PASS();
}

// 11. Confidence reflects annotation coverage
void test_confidence_levels() {
    TEST(confidence_levels);
    // Explicit annotation: highest confidence
    auto wi1 = makeItem("f1", "template", "local");
    auto d1 = engine.route(wi1);

    // No annotations, just defaults: lower confidence
    auto wi2 = makeItem("f2", "", "local");
    auto d2 = engine.route(wi2);

    CHECK(d1.confidence > d2.confidence,
          "explicit has higher confidence than default");
    PASS();
}

// 12. Context budget reasonable for each width
void test_context_budgets() {
    TEST(context_budgets);
    CHECK(estimateContextBudget("local") == 500, "local=500");
    CHECK(estimateContextBudget("file") == 2000, "file=2000");
    CHECK(estimateContextBudget("project") == 8000, "project=8000");
    CHECK(estimateContextBudget("cross-project") == 16000, "cross-project=16000");
    CHECK(estimateContextBudget("unknown") == 500, "unknown defaults to 500");
    PASS();
}

int main() {
    std::cout << "=== Step 326: RoutingEngine Annotation-to-Dispatch Logic ===\n";
    try {
        test_explicit_override();
        test_ambiguity_human();
        test_getter_deterministic();
        test_setter_deterministic();
        test_cross_project_llm();
        test_project_llm();
        test_review_annotation();
        test_batch_routing();
        test_default_local();
        test_default_file();
        test_confidence_levels();
        test_context_budgets();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
