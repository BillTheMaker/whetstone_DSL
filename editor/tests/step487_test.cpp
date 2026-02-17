// Step 487: Phase 23c Integration Tests (integration, regression)

#include "SelfHostingIntegration.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SelfHostingInput sampleInput() {
    SelfHostingInput in;
    in.repoRoot = ".";
    in.stepMarkdown =
        "Run convention extractor, context injector, step expander, and test plan generator "
        "for workflow pipeline dispatch validation.";
    in.stepNumber = "487";
    in.componentName = "SelfHostingIntegration";
    in.priorHeaders = {
        "editor/src/StepSpecExpander.h",
        "editor/src/ProjectConventionAnalyzer.h",
        "editor/src/PriorStepContextInjector.h",
        "editor/src/TestPlanGenerator.h"
    };
    in.knownGoodFiles = {
        {"editor/src/GoodStep.h", "struct GoodStep { int x; };"},
        {"editor/tests/step900_test.cpp",
         "#define CHECK(a,b) do{}while(0)\nint main(){ CHECK(true, \"ok\"); return 0; }"},
        {"editor/CMakeLists.txt", "add_executable(step900_test tests/step900_test.cpp)\n"}
    };
    in.knownBrokenFiles = {
        {"editor/src/bad_step.h", "struct bad_step { int x; };\n"},
        {"editor/tests/step901_test.cpp", "int main(){ return 0; }\n"},
        {"editor/CMakeLists.txt", "add_executable(other tests/other.cpp)\n"}
    };
    return in;
}

void test_flow_populates_conventions_context_spec_and_testplan() {
    TEST(flow_populates_conventions_context_spec_and_testplan);
    auto r = SelfHostingIntegration::run(sampleInput());
    CHECK(!r.context.priorApis.empty(), "missing context apis");
    CHECK(!r.spec.testPlan.empty(), "missing step spec test plan");
    CHECK(!r.testPlan.tests.empty(), "missing generated test plan");
    CHECK(r.conventions.buildSystem == "cmake", "missing conventions build system");
    PASS();
}

void test_worker_readiness_is_true_for_realistic_step_input() {
    TEST(worker_readiness_is_true_for_realistic_step_input);
    auto r = SelfHostingIntegration::run(sampleInput());
    CHECK(r.workerReady, "expected workerReady true");
    PASS();
}

void test_generated_test_plan_uses_pipeline_integration_heuristic() {
    TEST(generated_test_plan_uses_pipeline_integration_heuristic);
    auto r = SelfHostingIntegration::run(sampleInput());
    CHECK(r.testPlan.hasType(PlannedTestType::Integration), "expected integration tests");
    PASS();
}

void test_good_completed_step_has_zero_convention_violations() {
    TEST(good_completed_step_has_zero_convention_violations);
    auto r = SelfHostingIntegration::run(sampleInput());
    CHECK(r.goodReport.violations.empty(), "known-good step should have no violations");
    PASS();
}

void test_broken_step_triggers_convention_violations() {
    TEST(broken_step_triggers_convention_violations);
    auto r = SelfHostingIntegration::run(sampleInput());
    CHECK(!r.brokenReport.violations.empty(), "broken step should trigger violations");
    CHECK(r.brokenReport.hasViolation("CLASS_NAMING_MISMATCH"), "missing class naming violation");
    CHECK(r.brokenReport.hasViolation("TEST_ASSERTION_MISSING"), "missing assertion violation");
    CHECK(r.brokenReport.hasViolation("BUILD_TARGET_MISSING"), "missing cmake violation");
    PASS();
}

void test_notes_record_data_flow_and_sufficiency_status() {
    TEST(notes_record_data_flow_and_sufficiency_status);
    auto r = SelfHostingIntegration::run(sampleInput());
    CHECK(r.notes.size() >= 2, "missing notes");
    CHECK(r.notes[0].find("conventions -> context -> spec -> test plan") != std::string::npos,
          "missing flow note");
    PASS();
}

void test_context_includes_prior_step_api_summaries() {
    TEST(context_includes_prior_step_api_summaries);
    auto r = SelfHostingIntegration::run(sampleInput());
    bool foundStepSpec = false;
    for (const auto& api : r.context.priorApis) {
        if (api.name == "StepSpec") foundStepSpec = true;
    }
    CHECK(foundStepSpec, "expected StepSpec in context summaries");
    PASS();
}

void test_regression_step486_still_passes() {
    TEST(regression_step486_still_passes);
    auto r = SelfHostingIntegration::run(sampleInput());
    CHECK(r.testPlan.tests.size() >= 6, "unexpected plan shrinkage");
    CHECK(r.spec.buildEntry.targetName == "step487_test", "unexpected step build target");
    PASS();
}

int main() {
    std::cout << "Step 487: Phase 23c Integration Tests\n";

    test_flow_populates_conventions_context_spec_and_testplan();      // 1
    test_worker_readiness_is_true_for_realistic_step_input();         // 2
    test_generated_test_plan_uses_pipeline_integration_heuristic();   // 3
    test_good_completed_step_has_zero_convention_violations();        // 4
    test_broken_step_triggers_convention_violations();                // 5
    test_notes_record_data_flow_and_sufficiency_status();             // 6
    test_context_includes_prior_step_api_summaries();                 // 7
    test_regression_step486_still_passes();                           // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
