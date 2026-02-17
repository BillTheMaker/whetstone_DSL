// Step 581: Acceptance-Criteria Binding (12 tests)

#include "AcceptanceCriteriaBinding.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static AnnotatedTaskitem task(const std::string& id) {
    AnnotatedTaskitem t;
    t.base.taskId = id;
    t.base.title = "title";
    t.base.milestoneId = "m1";
    t.base.queueReady = true;
    t.confidence = 80;
    return t;
}

static NormalizedRequirement acceptance(const std::string& id, const std::string& text) {
    NormalizedRequirement r;
    r.requirementId = id;
    r.kind = NormalizedRequirementKind::Acceptance;
    r.normalizedText = text;
    r.anchor = "acceptance";
    r.sourceLine = 1;
    return r;
}

void test_bind_success() {
    TEST(bind_success);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1")}, {acceptance("acc-1", "all tests pass")}, &out, &error), "bind should succeed");
    CHECK(out.size() == 1, "bound task count mismatch");
    PASS();
}

void test_bind_fails_for_empty_tasks() {
    TEST(bind_fails_for_empty_tasks);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(!AcceptanceCriteriaBinding::bind({}, {acceptance("acc-1", "a")}, &out, &error), "bind should fail");
    CHECK(error == "tasks_empty", "wrong error");
    PASS();
}

void test_bind_fails_for_missing_acceptance_requirements() {
    TEST(bind_fails_for_missing_acceptance_requirements);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(!AcceptanceCriteriaBinding::bind({task("task-1")}, {}, &out, &error), "bind should fail");
    CHECK(error == "acceptance_requirements_empty", "wrong error");
    PASS();
}

void test_each_task_receives_all_acceptance_checks() {
    TEST(each_task_receives_all_acceptance_checks);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1"), task("task-2")},
                                          {acceptance("acc-1", "a"), acceptance("acc-2", "b")},
                                          &out, &error), "bind should succeed");
    CHECK(out[0].checks.size() == 2, "task-1 check count mismatch");
    CHECK(out[1].checks.size() == 2, "task-2 check count mismatch");
    PASS();
}

void test_check_ids_include_task_and_requirement_ids() {
    TEST(check_ids_include_task_and_requirement_ids);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1")},
                                          {acceptance("acc-7", "a")},
                                          &out, &error), "bind should succeed");
    CHECK(out[0].checks[0].checkId == "task-1::acc-7", "check id mismatch");
    PASS();
}

void test_check_text_matches_acceptance_text() {
    TEST(check_text_matches_acceptance_text);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1")},
                                          {acceptance("acc-1", "must pass integration")},
                                          &out, &error), "bind should succeed");
    CHECK(out[0].checks[0].text == "must pass integration", "check text mismatch");
    PASS();
}

void test_test_skeleton_is_generated() {
    TEST(test_skeleton_is_generated);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1")},
                                          {acceptance("acc-1", "must pass integration")},
                                          &out, &error), "bind should succeed");
    CHECK(out[0].checks[0].testSkeleton.find("test_task_1_") == 0, "test skeleton prefix mismatch");
    PASS();
}

void test_test_skeleton_sanitizes_symbols() {
    TEST(test_skeleton_sanitizes_symbols);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1")},
                                          {acceptance("acc-1", "must-pass, integration!")},
                                          &out, &error), "bind should succeed");
    CHECK(out[0].checks[0].testSkeleton == "test_task_1_must_pass_integration", "sanitized skeleton mismatch");
    PASS();
}

void test_has_coverage_true_when_checks_present() {
    TEST(has_coverage_true_when_checks_present);
    std::vector<BoundTaskitem> out;
    std::string error;
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1")},
                                          {acceptance("acc-1", "a")},
                                          &out, &error), "bind should succeed");
    CHECK(out[0].hasCoverage, "hasCoverage should be true");
    PASS();
}

void test_non_acceptance_requirements_are_ignored() {
    TEST(non_acceptance_requirements_are_ignored);
    std::vector<BoundTaskitem> out;
    std::string error;
    NormalizedRequirement r;
    r.requirementId = "goal-1";
    r.kind = NormalizedRequirementKind::Goal;
    r.normalizedText = "goal";
    CHECK(!AcceptanceCriteriaBinding::bind({task("task-1")}, {r}, &out, &error), "bind should fail");
    CHECK(error == "acceptance_requirements_empty", "wrong error");
    PASS();
}

void test_multiple_tasks_preserve_original_annotation_fields() {
    TEST(multiple_tasks_preserve_original_annotation_fields);
    std::vector<BoundTaskitem> out;
    std::string error;
    auto t1 = task("task-1");
    t1.confidence = 42;
    auto t2 = task("task-2");
    t2.confidence = 99;
    CHECK(AcceptanceCriteriaBinding::bind({t1, t2},
                                          {acceptance("acc-1", "a")},
                                          &out, &error), "bind should succeed");
    CHECK(out[0].task.confidence == 42, "task-1 confidence should be preserved");
    CHECK(out[1].task.confidence == 99, "task-2 confidence should be preserved");
    PASS();
}

void test_binding_scales_with_many_acceptance_checks() {
    TEST(binding_scales_with_many_acceptance_checks);
    std::vector<BoundTaskitem> out;
    std::string error;
    std::vector<NormalizedRequirement> reqs;
    for (int i = 0; i < 10; ++i) reqs.push_back(acceptance("acc-" + std::to_string(i), "check " + std::to_string(i)));
    CHECK(AcceptanceCriteriaBinding::bind({task("task-1")}, reqs, &out, &error), "bind should succeed");
    CHECK(out[0].checks.size() == 10, "all checks should bind");
    PASS();
}

int main() {
    std::cout << "Step 581: Acceptance-Criteria Binding\n";

    test_bind_success();                                 // 1
    test_bind_fails_for_empty_tasks();                   // 2
    test_bind_fails_for_missing_acceptance_requirements();// 3
    test_each_task_receives_all_acceptance_checks();     // 4
    test_check_ids_include_task_and_requirement_ids();   // 5
    test_check_text_matches_acceptance_text();           // 6
    test_test_skeleton_is_generated();                   // 7
    test_test_skeleton_sanitizes_symbols();              // 8
    test_has_coverage_true_when_checks_present();        // 9
    test_non_acceptance_requirements_are_ignored();      // 10
    test_multiple_tasks_preserve_original_annotation_fields(); // 11
    test_binding_scales_with_many_acceptance_checks();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
