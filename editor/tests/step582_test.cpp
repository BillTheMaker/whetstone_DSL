// Step 582: Intake-to-Queue Simulation Harness (12 tests)

#include "IntakeToQueueSimulationHarness.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string validMarkdown() {
    return R"(## Goals
- Improve intake quality

## Constraints
- Enable deterministic parser behavior

## Dependencies
- CMake

## Acceptance Criteria
- Queue includes bound acceptance checks
- Confidence annotations are present
)";
}

static bool hasNote(const IntakeQueueSimulationResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

void test_full_pipeline_success() {
    TEST(full_pipeline_success);
    IntakeQueueSimulationInput input;
    input.markdown = validMarkdown();
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(result.ok, "pipeline should succeed");
    CHECK(hasNote(result, "queue:ready"), "queue ready note expected");
    PASS();
}

void test_parse_failure() {
    TEST(parse_failure);
    IntakeQueueSimulationInput input;
    input.markdown = "";
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(!result.ok, "pipeline should fail");
    CHECK(hasNote(result, "fail:parse"), "parse failure note expected");
    PASS();
}

void test_normalize_failure() {
    TEST(normalize_failure);
    IntakeQueueSimulationInput input;
    input.markdown = "## Notes\ntext";
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(!result.ok, "pipeline should fail");
    CHECK(hasNote(result, "fail:normalize"), "normalize failure note expected");
    PASS();
}

void test_review_load_failure_when_constraints_missing() {
    TEST(review_load_failure_when_constraints_missing);
    IntakeQueueSimulationInput input;
    input.markdown = R"(## Goals
- Goal
## Acceptance Criteria
- Done
)";
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(!result.ok, "pipeline should fail");
    CHECK(hasNote(result, "fail:review_load"), "review load failure expected");
    PASS();
}

void test_modify_constraints_mode_still_succeeds() {
    TEST(modify_constraints_mode_still_succeeds);
    IntakeQueueSimulationInput input;
    input.markdown = validMarkdown();
    input.modifyConstraints = true;
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(result.ok, "pipeline should succeed");
    CHECK(result.queuedCount > 0, "queued count should be positive");
    PASS();
}

void test_queue_count_matches_queue_ready_items() {
    TEST(queue_count_matches_queue_ready_items);
    IntakeQueueSimulationInput input;
    input.markdown = validMarkdown();
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(result.ok, "pipeline should succeed");
    CHECK(result.queuedCount == result.queue.size(), "all queued items should be queue ready");
    PASS();
}

void test_bound_check_count_is_accumulated() {
    TEST(bound_check_count_is_accumulated);
    IntakeQueueSimulationInput input;
    input.markdown = validMarkdown();
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(result.ok, "pipeline should succeed");
    CHECK(result.boundCheckCount >= 2, "expected at least two bound checks");
    PASS();
}

void test_escalation_count_exposed() {
    TEST(escalation_count_exposed);
    IntakeQueueSimulationInput input;
    input.markdown = R"(## Goals
- Maybe improve quality somehow

## Constraints
- Enable parser

## Dependencies
- CMake

## Acceptance Criteria
- Done
)";
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(result.ok, "pipeline should succeed");
    CHECK(result.escalatedCount >= 1, "escalation count should be at least one");
    PASS();
}

void test_result_queue_contains_coverage() {
    TEST(result_queue_contains_coverage);
    IntakeQueueSimulationInput input;
    input.markdown = validMarkdown();
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(result.ok, "pipeline should succeed");
    CHECK(result.queue[0].hasCoverage, "bound task should have coverage");
    PASS();
}

void test_result_queue_contains_task_ids() {
    TEST(result_queue_contains_task_ids);
    IntakeQueueSimulationInput input;
    input.markdown = validMarkdown();
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(result.ok, "pipeline should succeed");
    CHECK(!result.queue[0].task.base.taskId.empty(), "task id should not be empty");
    PASS();
}

void test_acceptance_binding_failure_propagates() {
    TEST(acceptance_binding_failure_propagates);
    IntakeQueueSimulationInput input;
    input.markdown = R"(## Goals
- improve
## Constraints
- enable parser
## Dependencies
- cmake
)";
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(!result.ok, "pipeline should fail");
    CHECK(hasNote(result, "fail:bind_acceptance"), "bind failure note expected");
    PASS();
}

void test_queue_ready_note_only_on_success() {
    TEST(queue_ready_note_only_on_success);
    IntakeQueueSimulationInput input;
    input.markdown = "";
    auto result = IntakeToQueueSimulationHarness::run(input);
    CHECK(!hasNote(result, "queue:ready"), "queue note should not appear on failure");
    PASS();
}

int main() {
    std::cout << "Step 582: Intake-to-Queue Simulation Harness\n";

    test_full_pipeline_success();                     // 1
    test_parse_failure();                             // 2
    test_normalize_failure();                         // 3
    test_review_load_failure_when_constraints_missing();// 4
    test_modify_constraints_mode_still_succeeds();    // 5
    test_queue_count_matches_queue_ready_items();     // 6
    test_bound_check_count_is_accumulated();          // 7
    test_escalation_count_exposed();                  // 8
    test_result_queue_contains_coverage();            // 9
    test_result_queue_contains_task_ids();            // 10
    test_acceptance_binding_failure_propagates();     // 11
    test_queue_ready_note_only_on_success();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
