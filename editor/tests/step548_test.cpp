// Step 548: Phase 29a Integration (8 tests)

#include "Phase29aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Phase29aRequest baseRequest() {
    Phase29aRequest r;
    r.routing = {"rename", 0.9, true, true, false};
    r.context.taskitemId = "ti-548";
    r.context.contractTargets = {"Function"};
    r.context.contractSymbols = {"cursor"};
    r.context.contractOps = {"rename"};
    r.context.fileContext = {"file:main.cpp", "ast:Function cursor", "diag:none"};
    r.context.projectContext = {"module:cursor helpers", "module:theme"};
    r.context.dependencyContext = {"dep:cursor_utils", "dep:sdl2"};
    r.context.narrowOperation = true;

    r.baseConfidence = 0.8;
    r.preferredRoute = "deterministic_template";
    r.successes = {{"deterministic_template", 8}};
    r.attempts = {{"deterministic_template", 10}};
    r.postApplyFailures = {{"deterministic_template", 1}};

    r.cost = {120, 500, 200, 1000, false, false};
    return r;
}

static bool hasError(const Phase29aResult& r, const std::string& e) {
    for (const auto& x : r.errors) if (x == e) return true;
    return false;
}

void test_pass_path_with_lower_context_and_cost_compliance() {
    TEST(pass_path_with_lower_context_and_cost_compliance);
    auto r = Phase29aIntegration::run(baseRequest());
    CHECK(r.passed, "expected pass path");
    CHECK(r.costPolicy.allowed, "cost policy should allow");
    PASS();
}

void test_cost_policy_block_fails_integration() {
    TEST(cost_policy_block_fails_integration);
    auto req = baseRequest();
    req.cost = {400, 1500, 200, 1000, false, false};
    auto r = Phase29aIntegration::run(req);
    CHECK(!r.passed, "cost-blocked path should fail integration");
    CHECK(hasError(r, "cost_policy_blocked"), "cost error expected");
    PASS();
}

void test_routing_escalation_fails_integration() {
    TEST(routing_escalation_fails_integration);
    auto req = baseRequest();
    req.routing.hasRecentPostApplyFailures = true;
    auto r = Phase29aIntegration::run(req);
    CHECK(!r.passed, "routing escalation should fail integration");
    CHECK(hasError(r, "routing_escalated"), "routing escalation error expected");
    PASS();
}

void test_narrow_context_is_minimized() {
    TEST(narrow_context_is_minimized);
    auto req = baseRequest();
    auto r = Phase29aIntegration::run(req);
    CHECK(r.minimizedContext.removedContext.size() > 0, "narrow mode should remove irrelevant context");
    PASS();
}

void test_low_calibrated_confidence_blocks_template_route() {
    TEST(low_calibrated_confidence_blocks_template_route);
    auto req = baseRequest();
    req.baseConfidence = 0.2;
    req.successes = {{"deterministic_template", 0}};
    req.attempts = {{"deterministic_template", 10}};
    req.postApplyFailures = {{"deterministic_template", 8}};
    auto r = Phase29aIntegration::run(req);
    CHECK(!r.passed, "low calibrated confidence should block");
    CHECK(hasError(r, "low_calibrated_confidence_for_template_route"), "calibration error expected");
    PASS();
}

void test_calibration_can_shift_route_recommendation() {
    TEST(calibration_can_shift_route_recommendation);
    auto req = baseRequest();
    req.successes = {{"deterministic_template", 1}, {"constrained_worker", 9}};
    req.attempts = {{"deterministic_template", 10}, {"constrained_worker", 10}};
    req.postApplyFailures = {{"deterministic_template", 6}, {"constrained_worker", 0}};
    auto r = Phase29aIntegration::run(req);
    CHECK(r.calibrated.recommendedRoute == "constrained_worker", "calibration should prefer stronger route");
    PASS();
}

void test_cost_guard_allows_when_rationale_and_approval_present() {
    TEST(cost_guard_allows_when_rationale_and_approval_present);
    auto req = baseRequest();
    req.cost = {300, 1300, 200, 1000, true, true};
    auto r = Phase29aIntegration::run(req);
    CHECK(r.costPolicy.allowed, "approved overage should be allowed");
    PASS();
}

void test_phase29a_handles_non_narrow_context_without_minimization_error() {
    TEST(phase29a_handles_non_narrow_context_without_minimization_error);
    auto req = baseRequest();
    req.context.narrowOperation = false;
    auto r = Phase29aIntegration::run(req);
    CHECK(!hasError(r, "context_not_sufficiently_minimized"), "non-narrow mode should not trigger strict minimization error");
    PASS();
}

int main() {
    std::cout << "Step 548: Phase 29a Integration\n";

    test_pass_path_with_lower_context_and_cost_compliance();         // 1
    test_cost_policy_block_fails_integration();                      // 2
    test_routing_escalation_fails_integration();                     // 3
    test_narrow_context_is_minimized();                              // 4
    test_low_calibrated_confidence_blocks_template_route();          // 5
    test_calibration_can_shift_route_recommendation();               // 6
    test_cost_guard_allows_when_rationale_and_approval_present();    // 7
    test_phase29a_handles_non_narrow_context_without_minimization_error(); // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
