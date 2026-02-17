// Step 502: Scenario - Multi-Model Orchestration Tests (12 tests)

#include "MultiModelOrchestrationScenarioRunner.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static MultiModelOrchestrationScenarioResult runNow() {
    return MultiModelOrchestrationScenarioRunner::run();
}

void test_scenario_builds_exactly_twenty_tasks() {
    TEST(scenario_builds_exactly_twenty_tasks);
    auto r = runNow();
    CHECK(r.tasks.size() == 20, "expected 20 tasks");
    PASS();
}

void test_task_group_distribution_matches_spec() {
    TEST(task_group_distribution_matches_spec);
    auto r = runNow();
    CHECK(r.countGroup(MultiModelTaskGroup::Deterministic) == 5, "expected 5 deterministic");
    CHECK(r.countGroup(MultiModelTaskGroup::Template) == 5, "expected 5 template");
    CHECK(r.countGroup(MultiModelTaskGroup::SLM) == 5, "expected 5 slm");
    CHECK(r.countGroup(MultiModelTaskGroup::LLM) == 3, "expected 3 llm");
    CHECK(r.countGroup(MultiModelTaskGroup::Human) == 2, "expected 2 human");
    PASS();
}

void test_routed_worker_distribution_exercises_all_worker_paths() {
    TEST(routed_worker_distribution_exercises_all_worker_paths);
    auto r = runNow();
    CHECK(r.routedCounts["deterministic"] == 5, "expected 5 deterministic routes");
    CHECK(r.routedCounts["template"] == 5, "expected 5 template routes");
    CHECK(r.routedCounts["slm"] == 5, "expected 5 slm routes");
    CHECK(r.routedCounts["llm"] == 3, "expected 3 llm routes");
    CHECK(r.routedCounts["human"] == 2, "expected 2 human routes");
    PASS();
}

void test_deterministic_and_template_tasks_auto_complete() {
    TEST(deterministic_and_template_tasks_auto_complete);
    auto r = runNow();
    int autoDone = 0;
    for (const auto& t : r.tasks) {
        if (t.autoCompleted) ++autoDone;
    }
    CHECK(autoDone == 10, "expected 10 auto-completed deterministic/template tasks");
    PASS();
}

void test_slm_tasks_use_narrow_context_widths() {
    TEST(slm_tasks_use_narrow_context_widths);
    auto r = runNow();
    for (const auto& t : r.tasks) {
        if (t.group != MultiModelTaskGroup::SLM) continue;
        CHECK(t.route.contextWidth == "local" || t.route.contextWidth == "file",
              "slm tasks should use local/file context");
    }
    PASS();
}

void test_llm_tasks_use_wide_context_widths() {
    TEST(llm_tasks_use_wide_context_widths);
    auto r = runNow();
    for (const auto& t : r.tasks) {
        if (t.group != MultiModelTaskGroup::LLM) continue;
        CHECK(t.route.contextWidth == "project" || t.route.contextWidth == "cross-project",
              "llm tasks should use project/cross-project context");
    }
    PASS();
}

void test_human_tasks_are_flagged_for_review() {
    TEST(human_tasks_are_flagged_for_review);
    auto r = runNow();
    int humanReview = 0;
    for (const auto& t : r.tasks) {
        if (t.group == MultiModelTaskGroup::Human && t.reviewRequired) ++humanReview;
    }
    CHECK(humanReview == 2, "expected both human tasks to require review");
    PASS();
}

void test_progress_tracking_reaches_one_hundred_percent() {
    TEST(progress_tracking_reaches_one_hundred_percent);
    auto r = runNow();
    CHECK(!r.progress.empty(), "expected progress points");
    CHECK(r.progress.back().completed == 20, "expected all tasks completed");
    CHECK(r.progress.back().percent >= 100.0f, "expected 100% completion");
    PASS();
}

void test_estimated_cost_and_actual_cost_are_computed() {
    TEST(estimated_cost_and_actual_cost_are_computed);
    auto r = runNow();
    CHECK(r.estimatedTotalTokens > 0, "missing estimated token total");
    CHECK(r.actualTotalTokens >= 0, "missing actual token total");
    CHECK(r.actualTotalTokens < r.estimatedTotalTokens, "expected actual token use below estimate");
    PASS();
}

void test_review_gates_are_exercised() {
    TEST(review_gates_are_exercised);
    auto r = runNow();
    CHECK(r.reviewGatesExercised, "review gates should be exercised");
    CHECK(r.reviewGateCount >= 5, "expected llm + human review gates");
    PASS();
}

void test_all_tasks_finish_in_completed_state() {
    TEST(all_tasks_finish_in_completed_state);
    auto r = runNow();
    for (const auto& t : r.tasks) {
        CHECK(t.completed, "all scenario tasks should be completed");
    }
    PASS();
}

void test_scenario_notes_capture_multi_model_summary() {
    TEST(scenario_notes_capture_multi_model_summary);
    auto r = runNow();
    CHECK(r.notes.size() >= 2, "expected scenario notes");
    CHECK(r.notes[0].find("20 tasks") != std::string::npos, "missing task-count summary");
    PASS();
}

int main() {
    std::cout << "Step 502: Scenario - Multi-Model Orchestration Tests\n";

    test_scenario_builds_exactly_twenty_tasks();                          // 1
    test_task_group_distribution_matches_spec();                           // 2
    test_routed_worker_distribution_exercises_all_worker_paths();          // 3
    test_deterministic_and_template_tasks_auto_complete();                 // 4
    test_slm_tasks_use_narrow_context_widths();                            // 5
    test_llm_tasks_use_wide_context_widths();                              // 6
    test_human_tasks_are_flagged_for_review();                             // 7
    test_progress_tracking_reaches_one_hundred_percent();                  // 8
    test_estimated_cost_and_actual_cost_are_computed();                    // 9
    test_review_gates_are_exercised();                                     // 10
    test_all_tasks_finish_in_completed_state();                            // 11
    test_scenario_notes_capture_multi_model_summary();                     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
