// Step 544: Constrained Routing Ruleset Extension (12 tests)

#include "ConstrainedRoutingRulesetExtension.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasReason(const ConstrainedRoutingDecision& d, const std::string& reason) {
    for (const auto& r : d.reasons) if (r == reason) return true;
    return false;
}

void test_non_constrained_mode_routes_general_worker() {
    TEST(non_constrained_mode_routes_general_worker);
    auto d = ConstrainedRoutingRulesetExtension::decide({"rename", 0.9, true, false, false});
    CHECK(d.route == "general_worker", "should route general when constrained off");
    CHECK(hasReason(d, "constrained_mode_disabled"), "reason expected");
    PASS();
}

void test_recent_failures_force_escalation() {
    TEST(recent_failures_force_escalation);
    auto d = ConstrainedRoutingRulesetExtension::decide({"rename", 0.95, true, true, true});
    CHECK(d.route == "escalate", "recent failures should escalate");
    CHECK(hasReason(d, "recent_post_apply_failures"), "reason expected");
    PASS();
}

void test_high_confidence_with_template_promotes_deterministic_route() {
    TEST(high_confidence_with_template_promotes_deterministic_route);
    auto d = ConstrainedRoutingRulesetExtension::decide({"rename", 0.92, true, true, false});
    CHECK(d.route == "deterministic_template", "high confidence + template should promote deterministic route");
    PASS();
}

void test_high_confidence_without_template_routes_constrained_worker() {
    TEST(high_confidence_without_template_routes_constrained_worker);
    auto d = ConstrainedRoutingRulesetExtension::decide({"rename", 0.92, false, true, false});
    CHECK(d.route == "constrained_worker", "without template should route constrained worker");
    PASS();
}

void test_moderate_confidence_routes_constrained_worker() {
    TEST(moderate_confidence_routes_constrained_worker);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.70, true, true, false});
    CHECK(d.route == "constrained_worker", "moderate confidence should route constrained worker");
    PASS();
}

void test_low_confidence_routes_general_worker() {
    TEST(low_confidence_routes_general_worker);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.40, true, true, false});
    CHECK(d.route == "general_worker", "low confidence should route general worker");
    CHECK(hasReason(d, "low_legal_choice_confidence"), "reason expected");
    PASS();
}

void test_template_threshold_is_inclusive_at_085() {
    TEST(template_threshold_is_inclusive_at_085);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.85, true, true, false});
    CHECK(d.route == "deterministic_template", "0.85 should qualify for template route");
    PASS();
}

void test_constrained_worker_threshold_is_inclusive_at_060() {
    TEST(constrained_worker_threshold_is_inclusive_at_060);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.60, false, true, false});
    CHECK(d.route == "constrained_worker", "0.60 should qualify for constrained worker");
    PASS();
}

void test_escalation_precedence_over_template_promotion() {
    TEST(escalation_precedence_over_template_promotion);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.99, true, true, true});
    CHECK(d.route == "escalate", "escalation should take precedence over template promotion");
    PASS();
}

void test_confidence_propagates_for_constrained_worker_route() {
    TEST(confidence_propagates_for_constrained_worker_route);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.73, false, true, false});
    CHECK(d.route == "constrained_worker", "route mismatch");
    CHECK(d.confidence > 0.72 && d.confidence < 0.74, "confidence should match input for constrained route");
    PASS();
}

void test_confidence_propagates_for_template_route() {
    TEST(confidence_propagates_for_template_route);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.89, true, true, false});
    CHECK(d.route == "deterministic_template", "route mismatch");
    CHECK(d.confidence > 0.88 && d.confidence < 0.90, "confidence should match input for template route");
    PASS();
}

void test_reasons_include_template_and_confidence_tags() {
    TEST(reasons_include_template_and_confidence_tags);
    auto d = ConstrainedRoutingRulesetExtension::decide({"update", 0.89, true, true, false});
    CHECK(hasReason(d, "high_legal_choice_confidence"), "high confidence reason expected");
    CHECK(hasReason(d, "template_available"), "template reason expected");
    PASS();
}

int main() {
    std::cout << "Step 544: Constrained Routing Ruleset Extension\n";

    test_non_constrained_mode_routes_general_worker();             // 1
    test_recent_failures_force_escalation();                       // 2
    test_high_confidence_with_template_promotes_deterministic_route(); // 3
    test_high_confidence_without_template_routes_constrained_worker(); // 4
    test_moderate_confidence_routes_constrained_worker();          // 5
    test_low_confidence_routes_general_worker();                   // 6
    test_template_threshold_is_inclusive_at_085();                 // 7
    test_constrained_worker_threshold_is_inclusive_at_060();       // 8
    test_escalation_precedence_over_template_promotion();          // 9
    test_confidence_propagates_for_constrained_worker_route();     // 10
    test_confidence_propagates_for_template_route();               // 11
    test_reasons_include_template_and_confidence_tags();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
