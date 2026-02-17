// Step 496: Self-Annotated Workflow Tests (12 tests)

#include "SelfModernizationWorkflow.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ModernizationWorkflowReport runDefault() {
    return SelfModernizationWorkflow::createAndRun("editor/src/AnnotationValidator.h");
}

void test_workflow_targets_annotationvalidator_header() {
    TEST(workflow_targets_annotationvalidator_header);
    auto r = runDefault();
    CHECK(r.targetFile.find("AnnotationValidator.h") != std::string::npos, "wrong target file");
    PASS();
}

void test_default_rules_include_simple_complex_and_new_diagnostic_categories() {
    TEST(default_rules_include_simple_complex_and_new_diagnostic_categories);
    auto rules = SelfModernizationWorkflow::defaultRules();
    bool hasSimple = false, hasComplex = false, hasNewDiag = false;
    for (const auto& rule : rules) {
        if (rule.kind == ModernizationRuleKind::SimpleRule) hasSimple = true;
        if (rule.kind == ModernizationRuleKind::ComplexRule) hasComplex = true;
        if (rule.kind == ModernizationRuleKind::NewDiagnosticCode) hasNewDiag = true;
    }
    CHECK(hasSimple && hasComplex && hasNewDiag, "missing expected rule categories");
    PASS();
}

void test_simple_rules_route_to_deterministic() {
    TEST(simple_rules_route_to_deterministic);
    auto r = runDefault();
    bool found = false;
    for (const auto& t : r.tasks) if (t.routeTo == "deterministic") found = true;
    CHECK(found, "expected deterministic tasks");
    PASS();
}

void test_complex_rules_route_to_llm() {
    TEST(complex_rules_route_to_llm);
    auto r = runDefault();
    bool found = false;
    for (const auto& t : r.tasks) if (t.routeTo == "llm") found = true;
    CHECK(found, "expected llm tasks");
    PASS();
}

void test_new_diagnostic_code_rules_route_to_human_review() {
    TEST(new_diagnostic_code_rules_route_to_human_review);
    auto r = runDefault();
    bool found = false;
    for (const auto& t : r.tasks) {
        if (t.routeTo == "human") {
            found = true;
            CHECK(t.reviewRequired, "human routed task must require review");
        }
    }
    CHECK(found, "expected human-routed task");
    PASS();
}

void test_deterministic_execution_generates_nonempty_code() {
    TEST(deterministic_execution_generates_nonempty_code);
    auto r = runDefault();
    for (const auto& t : r.tasks) {
        if (t.routeTo == "deterministic") {
            CHECK(!t.generatedCode.empty(), "deterministic task missing generated code");
        }
    }
    PASS();
}

void test_deterministic_generated_code_parses_as_valid_cpp() {
    TEST(deterministic_generated_code_parses_as_valid_cpp);
    auto r = runDefault();
    for (const auto& t : r.tasks) {
        if (t.routeTo == "deterministic") {
            CHECK(t.outputValid, "deterministic output should parse");
        }
    }
    PASS();
}

void test_workflow_summary_counts_match_task_routing() {
    TEST(workflow_summary_counts_match_task_routing);
    auto r = runDefault();
    int det = 0, llm = 0, human = 0;
    for (const auto& t : r.tasks) {
        if (t.routeTo == "deterministic") det++;
        else if (t.routeTo == "llm") llm++;
        else if (t.routeTo == "human") human++;
    }
    CHECK(r.deterministicCount == det, "deterministic count mismatch");
    CHECK(r.llmCount == llm, "llm count mismatch");
    CHECK(r.humanCount == human, "human count mismatch");
    PASS();
}

void test_deterministic_valid_count_is_not_greater_than_deterministic_count() {
    TEST(deterministic_valid_count_is_not_greater_than_deterministic_count);
    auto r = runDefault();
    CHECK(r.deterministicValidCount <= r.deterministicCount, "invalid deterministic valid count");
    PASS();
}

void test_each_task_has_stable_task_id_and_rule_id() {
    TEST(each_task_has_stable_task_id_and_rule_id);
    auto r = runDefault();
    for (const auto& t : r.tasks) {
        CHECK(t.taskId.find("self-modernize-") == 0, "task id prefix mismatch");
        CHECK(!t.ruleId.empty(), "rule id missing");
    }
    PASS();
}

void test_rule_diagnostic_codes_use_e149x_range() {
    TEST(rule_diagnostic_codes_use_e149x_range);
    auto rules = SelfModernizationWorkflow::defaultRules();
    for (const auto& r : rules) {
        CHECK(r.targetDiagnosticCode.find("E149") == 0, "diagnostic code range mismatch");
    }
    PASS();
}

void test_report_contains_completion_note() {
    TEST(report_contains_completion_note);
    auto r = runDefault();
    CHECK(!r.notes.empty(), "missing notes");
    CHECK(r.notes[0].find("self-improvement") != std::string::npos, "missing completion note");
    PASS();
}

int main() {
    std::cout << "Step 496: Self-Annotated Workflow Tests\n";

    test_workflow_targets_annotationvalidator_header();                    // 1
    test_default_rules_include_simple_complex_and_new_diagnostic_categories(); // 2
    test_simple_rules_route_to_deterministic();                           // 3
    test_complex_rules_route_to_llm();                                    // 4
    test_new_diagnostic_code_rules_route_to_human_review();               // 5
    test_deterministic_execution_generates_nonempty_code();               // 6
    test_deterministic_generated_code_parses_as_valid_cpp();              // 7
    test_workflow_summary_counts_match_task_routing();                    // 8
    test_deterministic_valid_count_is_not_greater_than_deterministic_count(); // 9
    test_each_task_has_stable_task_id_and_rule_id();                      // 10
    test_rule_diagnostic_codes_use_e149x_range();                         // 11
    test_report_contains_completion_note();                               // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
