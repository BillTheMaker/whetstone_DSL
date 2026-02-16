// Step 424: Context Window Optimization Tests (12 tests)

#include "ContextWindowOptimizer.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string repeat(const std::string& s, int n) {
    std::string out;
    for (int i = 0; i < n; ++i) out += s;
    return out;
}

static ContextPack makePack() {
    ContextPack p;
    p.instructions = "Implement task safely.";
    p.localContext = "fn add(x,y) { return x+y; }";
    p.fileContext = repeat("file_line;\n", 400);
    p.projectContext = repeat("project_line;\n", 3000);
    return p;
}

void test_estimate_tokens_non_empty() {
    TEST(estimate_tokens_non_empty);
    CHECK(ContextWindowOptimizer::estimateTokens("abcd") == 1, "4 chars => 1 token");
    CHECK(ContextWindowOptimizer::estimateTokens("abcdefgh") == 2, "8 chars => 2 tokens");
    PASS();
}

void test_estimate_tokens_empty_zero() {
    TEST(estimate_tokens_empty_zero);
    CHECK(ContextWindowOptimizer::estimateTokens("") == 0, "empty should be zero");
    PASS();
}

void test_budget_for_context_window_has_floor() {
    TEST(budget_for_context_window_has_floor);
    CHECK(ContextWindowOptimizer::budgetForContextWindow(100) == 512, "budget floor 512");
    PASS();
}

void test_small_context_uses_local_scope() {
    TEST(small_context_uses_local_scope);
    auto p = makePack();
    ModelProfile m{"local-slm", 8000, 0, 0, "fast", {}};
    auto out = ContextWindowOptimizer::optimize(p, m);
    CHECK(out.scope == "local", "small context should select local scope");
    CHECK(out.context.find("Local Context:") != std::string::npos, "missing local context");
    PASS();
}

void test_medium_context_uses_file_scope() {
    TEST(medium_context_uses_file_scope);
    auto p = makePack();
    ModelProfile m{"mid", 32000, 0, 0, "medium", {}};
    auto out = ContextWindowOptimizer::optimize(p, m);
    CHECK(out.scope == "file", "medium context should select file scope");
    CHECK(out.context.find("File Context:") != std::string::npos, "missing file context");
    PASS();
}

void test_large_context_uses_project_scope() {
    TEST(large_context_uses_project_scope);
    auto p = makePack();
    ModelProfile m{"large", 200000, 0, 0, "slow", {}};
    auto out = ContextWindowOptimizer::optimize(p, m);
    CHECK(out.scope == "project", "large context should select project scope");
    CHECK(out.context.find("Project Context:") != std::string::npos, "missing project context");
    PASS();
}

void test_optimizer_preserves_instructions_prefix() {
    TEST(optimizer_preserves_instructions_prefix);
    auto p = makePack();
    ModelProfile m{"mid", 32000, 0, 0, "medium", {}};
    auto out = ContextWindowOptimizer::optimize(p, m);
    CHECK(out.context.find("Implement task safely.") == 0, "instructions should remain prefix");
    PASS();
}

void test_optimizer_truncates_when_over_budget() {
    TEST(optimizer_truncates_when_over_budget);
    auto p = makePack();
    p.localContext = repeat("local_payload_", 5000); // force local-scope overflow
    ModelProfile m{"tiny", 1, 0, 0, "fast", {}};
    auto out = ContextWindowOptimizer::optimize(p, m);
    CHECK(out.truncated, "expected truncation");
    CHECK(out.context.find("[truncated to fit model context window]") != std::string::npos,
          "missing truncation marker");
    PASS();
}

void test_optimizer_respects_budget_token_upper_bound() {
    TEST(optimizer_respects_budget_token_upper_bound);
    auto p = makePack();
    ModelProfile m{"tiny", 2000, 0, 0, "fast", {}};
    auto out = ContextWindowOptimizer::optimize(p, m);
    CHECK(out.tokenEstimate <= out.budgetTokens + 20, "token estimate should stay near budget");
    PASS();
}

void test_optimize_for_worker_uses_registry_mapping() {
    TEST(optimize_for_worker_uses_registry_mapping);
    auto p = makePack();
    ModelProfileRegistry reg;
    reg.setWorkerProfileMapping("slm", "local-slm");
    auto out = ContextWindowOptimizer::optimizeForWorker(p, "slm", reg);
    CHECK(out.scope == "local", "slm should map to local profile scope");
    PASS();
}

void test_optimize_for_worker_unknown_uses_fallback() {
    TEST(optimize_for_worker_unknown_uses_fallback);
    auto p = makePack();
    ModelProfileRegistry reg;
    auto out = ContextWindowOptimizer::optimizeForWorker(p, "mystery", reg);
    CHECK(!out.context.empty(), "fallback should still produce context");
    CHECK(out.budgetTokens >= 512, "fallback should have sane budget");
    PASS();
}

void test_project_scope_contains_file_and_local_sections() {
    TEST(project_scope_contains_file_and_local_sections);
    auto p = makePack();
    ModelProfile m{"large", 200000, 0, 0, "slow", {}};
    auto out = ContextWindowOptimizer::optimize(p, m);
    CHECK(out.context.find("File Context:") != std::string::npos, "project should include file section");
    CHECK(out.context.find("Local Context:") != std::string::npos, "project should include local section");
    PASS();
}

int main() {
    std::cout << "Step 424: Context Window Optimization Tests\n";

    test_estimate_tokens_non_empty();                    // 1
    test_estimate_tokens_empty_zero();                   // 2
    test_budget_for_context_window_has_floor();          // 3
    test_small_context_uses_local_scope();               // 4
    test_medium_context_uses_file_scope();               // 5
    test_large_context_uses_project_scope();             // 6
    test_optimizer_preserves_instructions_prefix();      // 7
    test_optimizer_truncates_when_over_budget();         // 8
    test_optimizer_respects_budget_token_upper_bound();  // 9
    test_optimize_for_worker_uses_registry_mapping();    // 10
    test_optimize_for_worker_unknown_uses_fallback();    // 11
    test_project_scope_contains_file_and_local_sections();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
