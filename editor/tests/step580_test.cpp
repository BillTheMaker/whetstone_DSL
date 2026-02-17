// Step 580: Taskitem Confidence + Ambiguity Annotation (12 tests)

#include "TaskitemConfidenceAmbiguity.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static GeneratedTaskitem task(const std::string& id, bool queueReady) {
    GeneratedTaskitem t;
    t.taskId = id;
    t.title = "title";
    t.milestoneId = "m1";
    t.queueReady = queueReady;
    t.prerequisiteOps = {"validate-intake"};
    return t;
}

static NormalizedRequirement req(const std::string& id, bool ambiguous) {
    NormalizedRequirement r;
    r.requirementId = id;
    r.kind = NormalizedRequirementKind::Constraint;
    r.normalizedText = "text";
    r.anchor = "constraints";
    r.sourceLine = 1;
    r.ambiguous = ambiguous;
    return r;
}

void test_annotate_success() {
    TEST(annotate_success);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", true)}, normalized, &out, &error), "annotate should succeed");
    CHECK(out.size() == 1, "annotated task count mismatch");
    PASS();
}

void test_annotate_fails_for_empty_tasks() {
    TEST(annotate_fails_for_empty_tasks);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    std::string error;
    CHECK(!TaskitemConfidenceAmbiguity::annotate({}, normalized, &out, &error), "annotate should fail");
    CHECK(error == "tasks_empty", "wrong error");
    PASS();
}

void test_queue_not_ready_reduces_confidence() {
    TEST(queue_not_ready_reduces_confidence);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", false)}, normalized, &out, &error), "annotate should succeed");
    CHECK(out[0].confidence < 80, "confidence should be reduced");
    PASS();
}

void test_ambiguous_requirements_reduce_confidence() {
    TEST(ambiguous_requirements_reduce_confidence);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("r1", true));
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", true)}, normalized, &out, &error), "annotate should succeed");
    CHECK(out[0].ambiguityCount == 1, "ambiguity count mismatch");
    CHECK(out[0].confidence <= 75, "confidence should be reduced by ambiguity");
    PASS();
}

void test_conflicts_reduce_confidence() {
    TEST(conflicts_reduce_confidence);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    normalized.conflicts.push_back({"a", "b", "constraint_contradiction", "detail"});
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", true)}, normalized, &out, &error), "annotate should succeed");
    CHECK(out[0].confidence <= 77, "confidence should be reduced by conflicts");
    PASS();
}

void test_dependency_presence_adds_reason() {
    TEST(dependency_presence_adds_reason);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    auto t = task("task-1", true);
    t.dependencyTaskIds.push_back("task-0");
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({t}, normalized, &out, &error), "annotate should succeed");
    bool found = false;
    for (const auto& r : out[0].reasons) if (r == "depends_on_prior_tasks") found = true;
    CHECK(found, "dependency reason expected");
    PASS();
}

void test_low_confidence_adds_reason() {
    TEST(low_confidence_adds_reason);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("r1", true));
    normalized.requirements.push_back(req("r2", true));
    normalized.requirements.push_back(req("r3", true));
    normalized.conflicts.push_back({"a", "b", "constraint_contradiction", "detail"});
    auto t = task("task-1", false);
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({t}, normalized, &out, &error), "annotate should succeed");
    bool found = false;
    for (const auto& r : out[0].reasons) if (r == "low_confidence") found = true;
    CHECK(found, "low confidence reason expected");
    PASS();
}

void test_high_confidence_adds_clear_path_reason() {
    TEST(high_confidence_adds_clear_path_reason);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    auto t = task("task-1", true);
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({t}, normalized, &out, &error), "annotate should succeed");
    CHECK(out[0].reasons[0] == "high_confidence_clear_path", "clear-path reason expected");
    PASS();
}

void test_escalate_when_queue_not_ready() {
    TEST(escalate_when_queue_not_ready);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", false)}, normalized, &out, &error), "annotate should succeed");
    CHECK(out[0].escalate, "should escalate when queue not ready");
    PASS();
}

void test_escalate_when_ambiguous() {
    TEST(escalate_when_ambiguous);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    normalized.requirements.push_back(req("r1", true));
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", true)}, normalized, &out, &error), "annotate should succeed");
    CHECK(out[0].escalate, "should escalate when ambiguous");
    PASS();
}

void test_no_escalation_for_high_confidence_clear_item() {
    TEST(no_escalation_for_high_confidence_clear_item);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", true)}, normalized, &out, &error), "annotate should succeed");
    CHECK(!out[0].escalate, "should not escalate");
    PASS();
}

void test_multiple_tasks_all_annotated() {
    TEST(multiple_tasks_all_annotated);
    std::vector<AnnotatedTaskitem> out;
    RequirementNormalizationResult normalized;
    std::string error;
    CHECK(TaskitemConfidenceAmbiguity::annotate({task("task-1", true), task("task-2", false)}, normalized, &out, &error), "annotate should succeed");
    CHECK(out.size() == 2, "two annotations expected");
    CHECK(out[1].escalate, "second should escalate");
    PASS();
}

int main() {
    std::cout << "Step 580: Taskitem Confidence + Ambiguity Annotation\n";

    test_annotate_success();                         // 1
    test_annotate_fails_for_empty_tasks();          // 2
    test_queue_not_ready_reduces_confidence();      // 3
    test_ambiguous_requirements_reduce_confidence();// 4
    test_conflicts_reduce_confidence();             // 5
    test_dependency_presence_adds_reason();         // 6
    test_low_confidence_adds_reason();              // 7
    test_high_confidence_adds_clear_path_reason();  // 8
    test_escalate_when_queue_not_ready();           // 9
    test_escalate_when_ambiguous();                 // 10
    test_no_escalation_for_high_confidence_clear_item(); // 11
    test_multiple_tasks_all_annotated();            // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
