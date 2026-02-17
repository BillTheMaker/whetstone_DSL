// Step 482: Step Spec Expander Tests (unit, negative, boundary)

#include "StepSpecExpander.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_classifies_trivial_for_small_utility_step() {
    TEST(classifies_trivial_for_small_utility_step);
    StepExpanderInput in;
    in.stepMarkdown = "Add a simple utility struct with one helper.";
    in.stepNumber = "482";
    in.componentName = "UtilitySpec";
    auto spec = StepSpecExpander::expand(in);
    CHECK(spec.complexity == StepComplexity::Trivial, "expected trivial complexity");
    CHECK(spec.testPlan.size() <= 4, "trivial test budget exceeded");
    PASS();
}

void test_classifies_pipeline_when_workflow_keywords_present() {
    TEST(classifies_pipeline_when_workflow_keywords_present);
    StepExpanderInput in;
    in.stepMarkdown = "Build pipeline workflow integration across tools.";
    in.stepNumber = "482";
    in.componentName = "PipelineSpec";
    auto spec = StepSpecExpander::expand(in);
    CHECK(spec.complexity == StepComplexity::Pipeline, "expected pipeline complexity");
    CHECK(spec.testPlan.size() >= 6, "pipeline minimum coverage not met");
    CHECK(spec.testPlan.size() <= 10, "pipeline maximum exceeded");
    PASS();
}

void test_includes_negative_tests_when_validation_terms_present() {
    TEST(includes_negative_tests_when_validation_terms_present);
    StepExpanderInput in;
    in.stepMarkdown = "Validate input and reject invalid shape.";
    in.stepNumber = "482";
    in.componentName = "ValidatorSpec";
    auto spec = StepSpecExpander::expand(in);
    bool foundNegative = false;
    for (const auto& t : spec.testPlan) {
        if (t.type == PlannedTestType::Negative) foundNegative = true;
    }
    CHECK(foundNegative, "expected negative test stubs");
    PASS();
}

void test_includes_boundary_tests_when_limit_terms_present() {
    TEST(includes_boundary_tests_when_limit_terms_present);
    StepExpanderInput in;
    in.stepMarkdown = "Compute score with max/min limit boundary checks.";
    in.stepNumber = "482";
    in.componentName = "ScoringSpec";
    auto spec = StepSpecExpander::expand(in);
    bool foundBoundary = false;
    for (const auto& t : spec.testPlan) {
        if (t.type == PlannedTestType::Boundary) foundBoundary = true;
    }
    CHECK(foundBoundary, "expected boundary test stub");
    PASS();
}

void test_generated_test_names_follow_step_prefix_convention() {
    TEST(generated_test_names_follow_step_prefix_convention);
    StepExpanderInput in;
    in.stepMarkdown = "Standard generator step.";
    in.stepNumber = "482";
    in.componentName = "NameSpec";
    auto spec = StepSpecExpander::expand(in);
    CHECK(!spec.testPlan.empty(), "missing test stubs");
    CHECK(spec.testPlan[0].name.find("test_step482_") == 0, "test name prefix mismatch");
    PASS();
}

void test_header_skeleton_contains_expected_api_outline() {
    TEST(header_skeleton_contains_expected_api_outline);
    StepExpanderInput in;
    in.stepMarkdown = "Generate a step spec.";
    in.stepNumber = "482";
    in.componentName = "StepSpecEngine";
    auto spec = StepSpecExpander::expand(in);
    CHECK(spec.headerSkeleton.fileName == "StepSpecEngine.h", "header filename mismatch");
    CHECK(spec.headerSkeleton.methods.size() >= 3, "missing method stubs");
    CHECK(spec.headerSkeleton.fields.size() >= 3, "missing field stubs");
    PASS();
}

void test_build_entry_uses_cmake_target_pattern() {
    TEST(build_entry_uses_cmake_target_pattern);
    StepExpanderInput in;
    in.stepMarkdown = "Any step text.";
    in.stepNumber = "482";
    in.componentName = "BuildSpec";
    in.buildSystem = "cmake";
    auto spec = StepSpecExpander::expand(in);
    CHECK(spec.buildEntry.targetName == "step482_test", "target naming mismatch");
    CHECK(spec.buildEntry.snippet.find("add_executable(step482_test") != std::string::npos,
          "cmake snippet mismatch");
    PASS();
}

void test_empty_markdown_emits_warning_negative_case() {
    TEST(empty_markdown_emits_warning_negative_case);
    StepExpanderInput in;
    in.stepMarkdown = "";
    in.stepNumber = "482";
    in.componentName = "WarnSpec";
    auto spec = StepSpecExpander::expand(in);
    CHECK(!spec.warnings.empty(), "expected warning for empty markdown");
    PASS();
}

void test_complex_budget_expands_to_at_least_ten_tests() {
    TEST(complex_budget_expands_to_at_least_ten_tests);
    StepExpanderInput in;
    in.stepMarkdown =
        "Orchestrate dispatch protocol to extract conventions, validate context, generate skeleton, and integrate RPC.";
    in.stepNumber = "482";
    in.componentName = "ComplexSpec";
    auto spec = StepSpecExpander::expand(in);
    CHECK(spec.complexity == StepComplexity::Complex, "expected complex classification");
    CHECK(spec.testPlan.size() >= 10, "complex minimum budget not met");
    CHECK(spec.testPlan.size() <= 15, "complex budget upper bound exceeded");
    PASS();
}

int main() {
    std::cout << "Step 482: Step Spec Expander Tests\n";

    test_classifies_trivial_for_small_utility_step();                // 1
    test_classifies_pipeline_when_workflow_keywords_present();       // 2
    test_includes_negative_tests_when_validation_terms_present();    // 3
    test_includes_boundary_tests_when_limit_terms_present();         // 4
    test_generated_test_names_follow_step_prefix_convention();       // 5
    test_header_skeleton_contains_expected_api_outline();            // 6
    test_build_entry_uses_cmake_target_pattern();                    // 7
    test_empty_markdown_emits_warning_negative_case();               // 8
    test_complex_budget_expands_to_at_least_ten_tests();            // 9

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
