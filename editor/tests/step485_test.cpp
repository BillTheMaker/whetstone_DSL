// Step 485: Worker Dispatch + Result Validation Tests (unit, integration, negative)

#include "WorkerDispatchValidator.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkerDispatchInput sampleInput() {
    WorkerDispatchInput in;
    in.stepMarkdown = "Orchestrate worker dispatch pipeline and validate outputs.";
    in.stepNumber = "485";
    in.componentName = "WorkerDispatchValidator";
    in.repoRoot = ".";
    in.priorHeaders = {
        "editor/src/StepSpecExpander.h",
        "editor/src/ProjectConventionAnalyzer.h",
        "editor/src/PriorStepContextInjector.h"
    };
    return in;
}

static SourceFileDraft goodHeader() {
    return {"editor/src/DispatchOutput.h", "struct DispatchOutput { int x; }; \n"};
}

static SourceFileDraft goodTest() {
    return {"editor/tests/step999_test.cpp",
            "#define CHECK(a,b) do{}while(0)\nint main(){ CHECK(true, \"ok\"); return 0; }\n"};
}

static SourceFileDraft goodCmake() {
    return {"editor/CMakeLists.txt",
            "add_executable(step999_test tests/step999_test.cpp)\n"};
}

void test_dispatch_prepares_spec_context_and_conventions() {
    TEST(dispatch_prepares_spec_context_and_conventions);
    WorkerSubmission s{{goodHeader(), goodTest(), goodCmake()}, true, true, ""};
    auto out = WorkerDispatchValidator::run(sampleInput(), {s});
    CHECK(!out.spec.testPlan.empty(), "expected non-empty test plan");
    CHECK(!out.context.priorApis.empty(), "expected prior api context");
    CHECK(out.conventions.buildSystem == "cmake", "expected cmake conventions");
    PASS();
}

void test_accepts_submission_when_build_tests_and_conventions_pass() {
    TEST(accepts_submission_when_build_tests_and_conventions_pass);
    WorkerSubmission s{{goodHeader(), goodTest(), goodCmake()}, true, true, ""};
    auto out = WorkerDispatchValidator::run(sampleInput(), {s});
    CHECK(out.completed, "expected completed dispatch");
    CHECK(!out.escalated, "did not expect escalation");
    CHECK(out.attemptsUsed == 1, "expected one attempt");
    PASS();
}

void test_retries_after_test_failure_then_succeeds() {
    TEST(retries_after_test_failure_then_succeeds);
    WorkerSubmission bad{{goodHeader(), goodTest(), goodCmake()}, true, false, "1 failing test"};
    WorkerSubmission good{{goodHeader(), goodTest(), goodCmake()}, true, true, ""};
    auto out = WorkerDispatchValidator::run(sampleInput(), {bad, good}, 3);
    CHECK(out.completed, "expected success after retry");
    CHECK(out.attemptsUsed == 2, "expected two attempts used");
    CHECK(out.attempts[0].status.find("tests_failed") == 0, "first status should be tests_failed");
    PASS();
}

void test_escalates_when_retry_budget_exhausted() {
    TEST(escalates_when_retry_budget_exhausted);
    WorkerSubmission bad{{goodHeader(), goodTest(), goodCmake()}, false, false, "build break"};
    auto out = WorkerDispatchValidator::run(sampleInput(), {bad, bad, bad}, 2);
    CHECK(!out.completed, "should not complete");
    CHECK(out.escalated, "expected escalation");
    CHECK(out.attemptsUsed == 2, "should stop at retry budget");
    CHECK(out.finalStatus.find("Retry budget exhausted") != std::string::npos,
          "expected retry budget status");
    PASS();
}

void test_convention_violations_are_reported_and_not_accepted() {
    TEST(convention_violations_are_reported_and_not_accepted);
    SourceFileDraft badTest{"editor/tests/step999_test.cpp", "int main(){ return 0; }\n"};
    WorkerSubmission s{{goodHeader(), badTest, goodCmake()}, true, true, ""};
    auto out = WorkerDispatchValidator::run(sampleInput(), {s}, 1);
    CHECK(!out.completed, "should not accept convention violations");
    CHECK(out.escalated, "should escalate for architect decision");
    CHECK(out.attempts[0].conventionReport.hasViolation("TEST_ASSERTION_MISSING"),
          "expected test assertion violation");
    PASS();
}

void test_build_failure_status_contains_error_output() {
    TEST(build_failure_status_contains_error_output);
    WorkerSubmission s{{goodHeader(), goodTest(), goodCmake()}, false, false, "linker error"};
    auto out = WorkerDispatchValidator::run(sampleInput(), {s}, 1);
    CHECK(out.attempts[0].status.find("build_failed") == 0, "missing build_failed status");
    CHECK(out.attempts[0].status.find("linker error") != std::string::npos,
          "missing error payload");
    PASS();
}

void test_no_submissions_immediately_escalates_negative_case() {
    TEST(no_submissions_immediately_escalates_negative_case);
    auto out = WorkerDispatchValidator::run(sampleInput(), {}, 3);
    CHECK(out.escalated, "expected escalation");
    CHECK(!out.completed, "should not complete");
    CHECK(out.finalStatus.find("No worker submissions") != std::string::npos,
          "missing no submission message");
    PASS();
}

void test_default_retry_budget_is_three() {
    TEST(default_retry_budget_is_three);
    WorkerSubmission bad{{goodHeader(), goodTest(), goodCmake()}, false, false, "x"};
    auto out = WorkerDispatchValidator::run(sampleInput(), {bad, bad, bad, bad});
    CHECK(out.retryBudget == 3, "default budget mismatch");
    CHECK(out.attemptsUsed == 3, "should use three attempts");
    PASS();
}

void test_worker_brief_contains_core_handoff_sections() {
    TEST(worker_brief_contains_core_handoff_sections);
    WorkerSubmission s{{goodHeader(), goodTest(), goodCmake()}, true, true, ""};
    auto out = WorkerDispatchValidator::run(sampleInput(), {s});
    auto brief = WorkerDispatchValidator::buildWorkerBrief(
        sampleInput(), out.spec, out.context, out.conventions);
    CHECK(brief.find("Complexity:") != std::string::npos, "missing complexity");
    CHECK(brief.find("Tests planned:") != std::string::npos, "missing test plan count");
    CHECK(brief.find("Context brief:") != std::string::npos, "missing context brief");
    PASS();
}

void test_acceptance_requires_both_build_and_tests_even_with_clean_conventions() {
    TEST(acceptance_requires_both_build_and_tests_even_with_clean_conventions);
    WorkerSubmission s{{goodHeader(), goodTest(), goodCmake()}, true, false, ""};
    auto out = WorkerDispatchValidator::run(sampleInput(), {s}, 1);
    CHECK(!out.completed, "must not complete when tests fail");
    CHECK(out.escalated, "expected escalation");
    PASS();
}

int main() {
    std::cout << "Step 485: Worker Dispatch + Result Validation Tests\n";

    test_dispatch_prepares_spec_context_and_conventions();                    // 1
    test_accepts_submission_when_build_tests_and_conventions_pass();          // 2
    test_retries_after_test_failure_then_succeeds();                          // 3
    test_escalates_when_retry_budget_exhausted();                             // 4
    test_convention_violations_are_reported_and_not_accepted();               // 5
    test_build_failure_status_contains_error_output();                        // 6
    test_no_submissions_immediately_escalates_negative_case();                // 7
    test_default_retry_budget_is_three();                                     // 8
    test_worker_brief_contains_core_handoff_sections();                       // 9
    test_acceptance_requires_both_build_and_tests_even_with_clean_conventions(); // 10

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
