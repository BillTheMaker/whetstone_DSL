// Step 486: Test Plan Generator Tests (unit, boundary, negative)

#include "TestPlanGenerator.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TestPlanInput baseInput(const std::string& text) {
    TestPlanInput in;
    in.stepMarkdown = text;
    in.stepNumber = "486";
    in.componentName = "TestPlanGenerator";
    return in;
}

void test_validation_keywords_add_negative_tests() {
    TEST(validation_keywords_add_negative_tests);
    auto plan = TestPlanGenerator::generate(baseInput("Validate and reject invalid requests."));
    CHECK(plan.hasType(PlannedTestType::Negative), "expected negative tests");
    PASS();
}

void test_pipeline_keywords_add_integration_tests() {
    TEST(pipeline_keywords_add_integration_tests);
    auto plan = TestPlanGenerator::generate(baseInput("Build workflow pipeline for dispatch."));
    CHECK(plan.complexity == StepComplexity::Pipeline, "expected pipeline complexity");
    CHECK(plan.hasType(PlannedTestType::Integration), "expected integration tests");
    CHECK(plan.tests.size() >= 6 && plan.tests.size() <= 10, "pipeline budget mismatch");
    PASS();
}

void test_rpc_mcp_keywords_add_smoke_tests() {
    TEST(rpc_mcp_keywords_add_smoke_tests);
    auto plan = TestPlanGenerator::generate(baseInput("Add MCP RPC handler and routing support."));
    CHECK(plan.hasType(PlannedTestType::Smoke), "expected smoke test");
    PASS();
}

void test_boundary_keywords_add_boundary_tests() {
    TEST(boundary_keywords_add_boundary_tests);
    auto plan = TestPlanGenerator::generate(baseInput("Compute score with max/min boundary limits."));
    CHECK(plan.hasType(PlannedTestType::Boundary), "expected boundary test");
    PASS();
}

void test_modification_keywords_add_regression_tests() {
    TEST(modification_keywords_add_regression_tests);
    auto plan = TestPlanGenerator::generate(baseInput("Modify existing scoring rules."));
    CHECK(plan.hasType(PlannedTestType::Regression), "expected regression test");
    PASS();
}

void test_prior_test_patterns_can_force_regression_tests() {
    TEST(prior_test_patterns_can_force_regression_tests);
    auto in = baseInput("Add parser utility.");
    in.priorTests.push_back({"editor/tests/step123_test.cpp", "regression coverage"});
    auto plan = TestPlanGenerator::generate(in);
    CHECK(plan.hasType(PlannedTestType::Regression), "expected regression from prior pattern");
    PASS();
}

void test_contract_keywords_add_contract_tests() {
    TEST(contract_keywords_add_contract_tests);
    auto plan = TestPlanGenerator::generate(baseInput("Enforce @Contract pre/post checks."));
    CHECK(plan.hasType(PlannedTestType::Contract), "expected contract test");
    PASS();
}

void test_property_keywords_add_property_tests() {
    TEST(property_keywords_add_property_tests);
    auto plan = TestPlanGenerator::generate(baseInput("Roundtrip serialize invariant for nodes."));
    CHECK(plan.hasType(PlannedTestType::Property), "expected property test");
    PASS();
}

void test_complex_hint_enforces_10_to_15_budget_boundary() {
    TEST(complex_hint_enforces_10_to_15_budget_boundary);
    auto in = baseInput("Simple line.");
    in.useComplexityHint = true;
    in.complexityHint = StepComplexity::Complex;
    auto plan = TestPlanGenerator::generate(in);
    CHECK(plan.tests.size() >= 10, "expected complex minimum");
    CHECK(plan.tests.size() <= 15, "expected complex maximum");
    PASS();
}

void test_test_names_use_step_prefix_convention() {
    TEST(test_names_use_step_prefix_convention);
    auto plan = TestPlanGenerator::generate(baseInput("Validate behavior."));
    CHECK(!plan.tests.empty(), "expected test stubs");
    CHECK(plan.tests[0].name.find("test_step486_") == 0, "unexpected test name prefix");
    PASS();
}

int main() {
    std::cout << "Step 486: Test Plan Generator Tests\n";

    test_validation_keywords_add_negative_tests();              // 1
    test_pipeline_keywords_add_integration_tests();            // 2
    test_rpc_mcp_keywords_add_smoke_tests();                   // 3
    test_boundary_keywords_add_boundary_tests();               // 4
    test_modification_keywords_add_regression_tests();         // 5
    test_prior_test_patterns_can_force_regression_tests();     // 6
    test_contract_keywords_add_contract_tests();               // 7
    test_property_keywords_add_property_tests();               // 8
    test_complex_hint_enforces_10_to_15_budget_boundary();     // 9
    test_test_names_use_step_prefix_convention();              // 10

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
