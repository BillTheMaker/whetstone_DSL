// Step 425: Batch Submission for Agent Tasks Tests (12 tests)

#include "BatchTaskSubmitter.h"

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
                                 const std::string& lang,
                                 const std::string& file,
                                 const std::string& prompt) {
    AgentTaskRequest t;
    t.itemId = id;
    t.workerType = worker;
    t.language = lang;
    t.bufferId = file;
    t.context = "shared context for " + file;
    t.prompt = prompt;
    return t;
}

void test_groups_tasks_by_worker_language_buffer() {
    TEST(groups_tasks_by_worker_language_buffer);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "do A"),
        makeTask("a2", "llm", "cpp", "a.cpp", "do B"),
        makeTask("b1", "llm", "cpp", "b.cpp", "do C")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.batches.size() == 2, "expected two groups");
    PASS();
}

void test_separates_by_worker_type() {
    TEST(separates_by_worker_type);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "p1"),
        makeTask("a2", "slm", "cpp", "a.cpp", "p2")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.batches.size() == 2, "different workers should not mix");
    PASS();
}

void test_separates_by_language() {
    TEST(separates_by_language);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "p1"),
        makeTask("a2", "llm", "python", "a.py", "p2")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.batches.size() == 2, "different languages should not mix");
    PASS();
}

void test_honors_max_batch_size() {
    TEST(honors_max_batch_size);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "p1"),
        makeTask("a2", "llm", "cpp", "a.cpp", "p2"),
        makeTask("a3", "llm", "cpp", "a.cpp", "p3"),
        makeTask("a4", "llm", "cpp", "a.cpp", "p4"),
        makeTask("a5", "llm", "cpp", "a.cpp", "p5")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 2);
    CHECK(plan.batches.size() == 3, "5 tasks with size 2 => 3 batches");
    CHECK(plan.batches[0].tasks.size() <= 2, "batch 0 exceeds max");
    PASS();
}

void test_batch_keeps_shared_context_once() {
    TEST(batch_keeps_shared_context_once);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "p1"),
        makeTask("a2", "llm", "cpp", "a.cpp", "p2")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.batches.size() == 1, "expected one batch");
    CHECK(plan.batches[0].sharedContext.find("a.cpp") != std::string::npos,
          "shared context not preserved");
    PASS();
}

void test_batch_payload_json_contains_tasks() {
    TEST(batch_payload_json_contains_tasks);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "p1"),
        makeTask("a2", "llm", "cpp", "a.cpp", "p2")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    auto j = plan.batches[0].toJson();
    CHECK(j.contains("tasks"), "missing tasks json");
    CHECK((int)j["tasks"].size() == 2, "task count mismatch");
    PASS();
}

void test_deterministic_item_order_in_batch() {
    TEST(deterministic_item_order_in_batch);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("z2", "llm", "cpp", "a.cpp", "p2"),
        makeTask("a1", "llm", "cpp", "a.cpp", "p1")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.batches[0].tasks[0].itemId == "a1", "tasks should be sorted by itemId");
    PASS();
}

void test_empty_input_produces_empty_plan() {
    TEST(empty_input_produces_empty_plan);
    auto plan = BatchTaskSubmitter::planBatches({}, 4);
    CHECK(plan.batches.empty(), "expected no batches");
    CHECK(plan.estimatedTokensIndividual == 0, "expected zero individual tokens");
    PASS();
}

void test_max_batch_size_floor_to_one() {
    TEST(max_batch_size_floor_to_one);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "p1"),
        makeTask("a2", "llm", "cpp", "a.cpp", "p2")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 0);
    CHECK(plan.batches.size() == 2, "size<=0 should behave as maxBatchSize=1");
    PASS();
}

void test_estimated_tokens_batched_not_exceed_individual() {
    TEST(estimated_tokens_batched_not_exceed_individual);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "rewrite alpha"),
        makeTask("a2", "llm", "cpp", "a.cpp", "rewrite beta"),
        makeTask("a3", "llm", "cpp", "a.cpp", "rewrite gamma")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.estimatedTokensBatched <= plan.estimatedTokensIndividual,
          "batched should not exceed individual");
    PASS();
}

void test_savings_percent_positive_for_shared_context_batch() {
    TEST(savings_percent_positive_for_shared_context_batch);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "rewrite alpha"),
        makeTask("a2", "llm", "cpp", "a.cpp", "rewrite beta"),
        makeTask("a3", "llm", "cpp", "a.cpp", "rewrite gamma")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.estimatedSavingsPercent() > 0.0, "expected positive savings");
    PASS();
}

void test_multiple_groups_compute_savings_without_crash() {
    TEST(multiple_groups_compute_savings_without_crash);
    std::vector<AgentTaskRequest> tasks = {
        makeTask("a1", "llm", "cpp", "a.cpp", "p1"),
        makeTask("a2", "llm", "cpp", "a.cpp", "p2"),
        makeTask("b1", "llm", "python", "b.py", "p3"),
        makeTask("c1", "slm", "cpp", "c.cpp", "p4")
    };
    auto plan = BatchTaskSubmitter::planBatches(tasks, 8);
    CHECK(plan.batches.size() >= 3, "expected multiple groups");
    CHECK(plan.estimatedTokensIndividual >= plan.estimatedTokensBatched,
          "individual should be >= batched");
    PASS();
}

int main() {
    std::cout << "Step 425: Batch Submission for Agent Tasks Tests\n";

    test_groups_tasks_by_worker_language_buffer();            // 1
    test_separates_by_worker_type();                          // 2
    test_separates_by_language();                             // 3
    test_honors_max_batch_size();                             // 4
    test_batch_keeps_shared_context_once();                   // 5
    test_batch_payload_json_contains_tasks();                 // 6
    test_deterministic_item_order_in_batch();                 // 7
    test_empty_input_produces_empty_plan();                   // 8
    test_max_batch_size_floor_to_one();                       // 9
    test_estimated_tokens_batched_not_exceed_individual();    // 10
    test_savings_percent_positive_for_shared_context_batch(); // 11
    test_multiple_groups_compute_savings_without_crash();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
