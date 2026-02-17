// Step 427: Phase 18b Integration + Sprint Summary Tests (8 tests)

#include "ModelProfileRegistry.h"
#include "ContextWindowOptimizer.h"
#include "BatchTaskSubmitter.h"
#include "WorkflowCostTracker.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static AgentTaskRequest makeTask(const std::string& id,
                                 const std::string& worker,
                                 const std::string& file,
                                 const std::string& prompt,
                                 const std::string& ctx) {
    AgentTaskRequest t;
    t.itemId = id;
    t.workerType = worker;
    t.language = file.find(".py") != std::string::npos ? "python" : "cpp";
    t.bufferId = file;
    t.prompt = prompt;
    t.context = ctx;
    return t;
}

void test_phase18b_profile_context_batch_cost_pipeline() {
    TEST(phase18b_profile_context_batch_cost_pipeline);
    ModelProfileRegistry reg;
    ContextPack pack;
    pack.instructions = "Implement safely.";
    pack.localContext = "fn local(){}";
    pack.fileContext = std::string(5000, 'f');
    pack.projectContext = std::string(20000, 'p');
    auto optimized = ContextWindowOptimizer::optimizeForWorker(pack, "llm", reg);
    CHECK(!optimized.context.empty(), "optimized context missing");

    std::vector<AgentTaskRequest> tasks = {
        makeTask("w1", "llm", "a.cpp", "rewrite A", optimized.context),
        makeTask("w2", "llm", "a.cpp", "rewrite B", optimized.context)
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 4);
    CHECK(plan.batches.size() == 1, "expected one batch");

    WorkflowCostTracker costs(&reg);
    costs.recordEstimate("w1", "llm", 1200, 300);
    costs.recordEstimate("w2", "llm", 1100, 280);
    costs.recordActual("w1", "llm", 1250, 320);
    costs.recordActual("w2", "llm", 1000, 260);
    auto summary = costs.summarize();
    CHECK(summary.actualCost >= 0.0, "invalid actual cost");
    PASS();
}

void test_profile_mapping_affects_context_scope_choice() {
    TEST(profile_mapping_affects_context_scope_choice);
    ModelProfileRegistry reg;
    reg.setWorkerProfileMapping("llm", "local-slm");
    ContextPack pack;
    pack.instructions = "Do task";
    pack.localContext = "local";
    pack.fileContext = std::string(2000, 'f');
    pack.projectContext = std::string(10000, 'p');
    auto out = ContextWindowOptimizer::optimizeForWorker(pack, "llm", reg);
    CHECK(out.scope == "local", "remapped llm should choose local scope");
    PASS();
}

void test_batching_reduces_estimated_tokens_for_shared_context() {
    TEST(batching_reduces_estimated_tokens_for_shared_context);
    std::string shared = std::string(8000, 'x');
    std::vector<AgentTaskRequest> tasks = {
        makeTask("w1", "llm", "a.cpp", "task A", shared),
        makeTask("w2", "llm", "a.cpp", "task B", shared),
        makeTask("w3", "llm", "a.cpp", "task C", shared)
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.estimatedTokensBatched < plan.estimatedTokensIndividual,
          "expected batching token savings");
    PASS();
}

void test_cost_tracker_report_has_worker_and_profile_breakdown() {
    TEST(cost_tracker_report_has_worker_and_profile_breakdown);
    ModelProfileRegistry reg;
    WorkflowCostTracker costs(&reg);
    costs.recordActual("a", "llm", 1000, 100);
    costs.recordActual("b", "slm", 1000, 100);
    auto report = costs.reportJson();
    CHECK(report["costByWorker"].contains("llm"), "missing llm worker bucket");
    CHECK(report["costByProfile"].contains("claude-sonnet"), "missing sonnet profile bucket");
    PASS();
}

void test_context_optimizer_budget_alignment_with_profile() {
    TEST(context_optimizer_budget_alignment_with_profile);
    ContextPack pack;
    pack.instructions = "i";
    pack.localContext = std::string(5000, 'l');
    ModelProfile small{"small", 8000, 0, 0, "fast", {}};
    ModelProfile large{"large", 200000, 0, 0, "slow", {}};
    auto a = ContextWindowOptimizer::optimize(pack, small);
    auto b = ContextWindowOptimizer::optimize(pack, large);
    CHECK(a.budgetTokens < b.budgetTokens, "small model should have smaller budget");
    PASS();
}

void test_cost_variance_tracks_over_under_runs() {
    TEST(cost_variance_tracks_over_under_runs);
    ModelProfileRegistry reg;
    WorkflowCostTracker costs(&reg);
    costs.recordEstimate("w1", "llm", 1000, 100);
    costs.recordActual("w1", "llm", 1200, 150);
    costs.recordEstimate("w2", "llm", 1000, 100);
    costs.recordActual("w2", "llm", 800, 80);
    auto s = costs.summarize();
    CHECK(s.varianceCost() != 0.0, "variance should capture estimate drift");
    PASS();
}

void test_multigroup_batch_plan_is_deterministic() {
    TEST(multigroup_batch_plan_is_deterministic);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("w3", "slm", "b.py", "p3", "ctx-b"),
        makeTask("w1", "llm", "a.cpp", "p1", "ctx-a"),
        makeTask("w2", "llm", "a.cpp", "p2", "ctx-a")
    };
    auto p1 = BatchTaskSubmitter::planBatches(tasks, 8);
    auto p2 = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(p1.batches.size() == p2.batches.size(), "batch count should be deterministic");
    CHECK(p1.batches[0].workerType == p2.batches[0].workerType, "batch ordering drift");
    PASS();
}

void test_phase18b_expected_defaults_present() {
    TEST(phase18b_expected_defaults_present);
    ModelProfileRegistry reg;
    CHECK(reg.resolveProfileForWorker("llm") == "claude-sonnet", "llm default mapping mismatch");
    CHECK(reg.resolveProfileForWorker("slm") == "claude-haiku", "slm default mapping mismatch");
    PASS();
}

int main() {
    std::cout << "Step 427: Phase 18b Integration + Sprint Summary Tests\n";

    test_phase18b_profile_context_batch_cost_pipeline();         // 1
    test_profile_mapping_affects_context_scope_choice();         // 2
    test_batching_reduces_estimated_tokens_for_shared_context(); // 3
    test_cost_tracker_report_has_worker_and_profile_breakdown(); // 4
    test_context_optimizer_budget_alignment_with_profile();       // 5
    test_cost_variance_tracks_over_under_runs();                 // 6
    test_multigroup_batch_plan_is_deterministic();               // 7
    test_phase18b_expected_defaults_present();                   // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
