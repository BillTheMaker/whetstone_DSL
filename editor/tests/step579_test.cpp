// Step 579: Taskitem Generator v2 (12 tests)

#include "TaskitemGeneratorV2.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static DecomposedWorkstream ws(const std::string& id,
                               const std::string& title,
                               int uncertainty,
                               const std::vector<std::string>& reqIds) {
    DecomposedWorkstream w;
    w.workstreamId = id;
    w.title = title;
    w.uncertaintyScore = uncertainty;
    w.requirementIds = reqIds;
    return w;
}

void test_generate_success() {
    TEST(generate_success);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.title = "M1";
    m.workstreams.push_back(ws("ws1", "Primary", 10, {"r1"}));
    plan.milestones.push_back(m);

    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    CHECK(tasks.size() == 1, "task count mismatch");
    PASS();
}

void test_generate_fails_for_empty_plan() {
    TEST(generate_fails_for_empty_plan);
    DecomposedScopePlan plan;
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(!TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should fail");
    CHECK(error == "plan_empty", "wrong error");
    PASS();
}

void test_task_ids_increment() {
    TEST(task_ids_increment);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.workstreams.push_back(ws("ws1", "A", 10, {"r1"}));
    m.workstreams.push_back(ws("ws2", "B", 10, {"r2"}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    CHECK(tasks[0].taskId == "task-1", "first task id mismatch");
    CHECK(tasks[1].taskId == "task-2", "second task id mismatch");
    PASS();
}

void test_task_title_and_milestone_mapped() {
    TEST(task_title_and_milestone_mapped);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "milestone-77";
    m.workstreams.push_back(ws("ws1", "Execution Readiness Primary", 10, {"r1"}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    CHECK(tasks[0].title == "Execution Readiness Primary", "title mismatch");
    CHECK(tasks[0].milestoneId == "milestone-77", "milestone mismatch");
    PASS();
}

void test_queue_ready_requires_prereqs_and_requirements() {
    TEST(queue_ready_requires_prereqs_and_requirements);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.workstreams.push_back(ws("ws1", "Primary", 10, {}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    CHECK(!tasks[0].queueReady, "queueReady should be false with no requirements");
    PASS();
}

void test_uncertainty_adds_architect_review_prereq() {
    TEST(uncertainty_adds_architect_review_prereq);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.workstreams.push_back(ws("ws1", "Primary", 50, {"r1"}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    bool found = false;
    for (const auto& op : tasks[0].prerequisiteOps) if (op == "architect-review") found = true;
    CHECK(found, "architect-review prerequisite expected");
    PASS();
}

void test_execution_title_adds_dependency_resolution_prereq() {
    TEST(execution_title_adds_dependency_resolution_prereq);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.workstreams.push_back(ws("ws1", "Execution Ready Stream", 10, {"r1"}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    bool found = false;
    for (const auto& op : tasks[0].prerequisiteOps) if (op == "resolve-dependencies") found = true;
    CHECK(found, "resolve-dependencies prerequisite expected");
    PASS();
}

void test_review_title_adds_manual_approval_prereq() {
    TEST(review_title_adds_manual_approval_prereq);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.workstreams.push_back(ws("ws1", "Architect Review", 10, {"r1"}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    bool found = false;
    for (const auto& op : tasks[0].prerequisiteOps) if (op == "manual-approval") found = true;
    CHECK(found, "manual-approval prerequisite expected");
    PASS();
}

void test_second_milestone_task_depends_on_prior_task() {
    TEST(second_milestone_task_depends_on_prior_task);
    DecomposedScopePlan plan;
    DecomposedMilestone a;
    a.milestoneId = "m1";
    a.workstreams.push_back(ws("ws1", "A", 10, {"r1"}));
    DecomposedMilestone b;
    b.milestoneId = "m2";
    b.workstreams.push_back(ws("ws2", "B", 10, {"r2"}));
    plan.milestones.push_back(a);
    plan.milestones.push_back(b);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    CHECK(tasks[1].dependencyTaskIds.size() == 1, "dependency expected");
    CHECK(tasks[1].dependencyTaskIds[0] == tasks[0].taskId, "dependency id mismatch");
    PASS();
}

void test_tasks_empty_error_when_milestones_without_workstreams() {
    TEST(tasks_empty_error_when_milestones_without_workstreams);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(!TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should fail");
    CHECK(error == "tasks_empty", "wrong error");
    PASS();
}

void test_validate_intake_prerequisite_always_present() {
    TEST(validate_intake_prerequisite_always_present);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.workstreams.push_back(ws("ws1", "Any", 0, {"r1"}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    CHECK(!tasks[0].prerequisiteOps.empty(), "prerequisite ops expected");
    CHECK(tasks[0].prerequisiteOps[0] == "validate-intake", "validate-intake should be first");
    PASS();
}

void test_multiple_workstreams_generate_multiple_tasks() {
    TEST(multiple_workstreams_generate_multiple_tasks);
    DecomposedScopePlan plan;
    DecomposedMilestone m;
    m.milestoneId = "m1";
    m.workstreams.push_back(ws("ws1", "A", 10, {"r1"}));
    m.workstreams.push_back(ws("ws2", "B", 10, {"r2"}));
    m.workstreams.push_back(ws("ws3", "C", 10, {"r3"}));
    plan.milestones.push_back(m);
    std::vector<GeneratedTaskitem> tasks;
    std::string error;
    CHECK(TaskitemGeneratorV2::generate(plan, &tasks, &error), "generate should succeed");
    CHECK(tasks.size() == 3, "three tasks expected");
    PASS();
}

int main() {
    std::cout << "Step 579: Taskitem Generator v2\n";

    test_generate_success();                                // 1
    test_generate_fails_for_empty_plan();                  // 2
    test_task_ids_increment();                              // 3
    test_task_title_and_milestone_mapped();                // 4
    test_queue_ready_requires_prereqs_and_requirements();  // 5
    test_uncertainty_adds_architect_review_prereq();       // 6
    test_execution_title_adds_dependency_resolution_prereq();// 7
    test_review_title_adds_manual_approval_prereq();       // 8
    test_second_milestone_task_depends_on_prior_task();    // 9
    test_tasks_empty_error_when_milestones_without_workstreams();// 10
    test_validate_intake_prerequisite_always_present();    // 11
    test_multiple_workstreams_generate_multiple_tasks();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
