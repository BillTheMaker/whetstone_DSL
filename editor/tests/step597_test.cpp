// Step 597: Guardrail Drift Monitor (12 tests)

#include "GuardrailDriftMonitor.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static PolicyGuardrailCatalog catalog() {
    PolicyGuardrailCatalog c;
    std::string error;
    c.addRule({"allow-push", "git push", GuardrailDecision::Allow, "safe"}, &error);
    c.addRule({"review-deploy", "deploy", GuardrailDecision::RequireReview, "review"}, &error);
    c.addRule({"deny-rm", "rm -rf", GuardrailDecision::Deny, "deny"}, &error);
    return c;
}

void test_detect_no_drift_when_outcomes_match() {
    TEST(detect_no_drift_when_outcomes_match);
    auto c = catalog();
    std::vector<DriftObservation> obs = {
        {"git push origin", "allow"},
        {"deploy prod", "require_review"},
        {"rm -rf /tmp/x", "deny"}
    };
    CHECK(GuardrailDriftMonitor::detect(c, obs).empty(), "no drift expected");
    PASS();
}

void test_detect_drift_for_allow_mismatch() {
    TEST(detect_drift_for_allow_mismatch);
    auto c = catalog();
    std::vector<DriftObservation> obs = {
        {"git push origin", "require_review"}
    };
    auto f = GuardrailDriftMonitor::detect(c, obs);
    CHECK(f.size() == 1, "one drift expected");
    CHECK(f[0].expectedPolicy == "allow", "expected policy mismatch");
    PASS();
}

void test_detect_drift_for_review_mismatch() {
    TEST(detect_drift_for_review_mismatch);
    auto c = catalog();
    std::vector<DriftObservation> obs = {
        {"deploy prod", "allow"}
    };
    auto f = GuardrailDriftMonitor::detect(c, obs);
    CHECK(f.size() == 1, "one drift expected");
    CHECK(f[0].expectedPolicy == "require_review", "expected policy mismatch");
    PASS();
}

void test_detect_drift_for_deny_mismatch() {
    TEST(detect_drift_for_deny_mismatch);
    auto c = catalog();
    std::vector<DriftObservation> obs = {
        {"rm -rf /tmp/x", "allow"}
    };
    auto f = GuardrailDriftMonitor::detect(c, obs);
    CHECK(f.size() == 1, "one drift expected");
    CHECK(f[0].expectedPolicy == "deny", "expected policy mismatch");
    PASS();
}

void test_default_policy_drift_detected() {
    TEST(default_policy_drift_detected);
    auto c = catalog();
    std::vector<DriftObservation> obs = {
        {"unknown op", "allow"}
    };
    auto f = GuardrailDriftMonitor::detect(c, obs);
    CHECK(f.size() == 1, "default drift expected");
    CHECK(f[0].expectedPolicy == "require_review", "default expected policy mismatch");
    PASS();
}

void test_drift_score_zero_when_no_observations() {
    TEST(drift_score_zero_when_no_observations);
    CHECK(GuardrailDriftMonitor::driftScore({}, 0) == 0, "zero observations score should be zero");
    PASS();
}

void test_drift_score_percentage() {
    TEST(drift_score_percentage);
    std::vector<DriftFinding> f(2);
    CHECK(GuardrailDriftMonitor::driftScore(f, 8) == 25, "drift score should be 25");
    PASS();
}

void test_drift_score_capped_at_100() {
    TEST(drift_score_capped_at_100);
    std::vector<DriftFinding> f(10);
    CHECK(GuardrailDriftMonitor::driftScore(f, 5) == 100, "score cap mismatch");
    PASS();
}

void test_remediation_hint_for_deny_drift() {
    TEST(remediation_hint_for_deny_drift);
    std::vector<DriftFinding> f = {{"rm -rf /tmp/x", "deny", "allow"}};
    auto hints = GuardrailDriftMonitor::remediationHints(f);
    CHECK(hints.size() == 1, "hint count mismatch");
    CHECK(hints[0].find("enforce_hard_block:") == 0, "deny hint mismatch");
    PASS();
}

void test_remediation_hint_for_review_drift() {
    TEST(remediation_hint_for_review_drift);
    std::vector<DriftFinding> f = {{"deploy prod", "require_review", "allow"}};
    auto hints = GuardrailDriftMonitor::remediationHints(f);
    CHECK(hints[0].find("restore_review_gate:") == 0, "review hint mismatch");
    PASS();
}

void test_remediation_hint_for_allow_drift() {
    TEST(remediation_hint_for_allow_drift);
    std::vector<DriftFinding> f = {{"git push origin", "allow", "require_review"}};
    auto hints = GuardrailDriftMonitor::remediationHints(f);
    CHECK(hints[0].find("align_allow_path:") == 0, "allow hint mismatch");
    PASS();
}

void test_multiple_findings_generate_multiple_hints() {
    TEST(multiple_findings_generate_multiple_hints);
    std::vector<DriftFinding> f = {
        {"deploy prod", "require_review", "allow"},
        {"rm -rf /tmp/x", "deny", "allow"}
    };
    auto hints = GuardrailDriftMonitor::remediationHints(f);
    CHECK(hints.size() == 2, "two hints expected");
    PASS();
}

int main() {
    std::cout << "Step 597: Guardrail Drift Monitor\n";

    test_detect_no_drift_when_outcomes_match();      // 1
    test_detect_drift_for_allow_mismatch();          // 2
    test_detect_drift_for_review_mismatch();         // 3
    test_detect_drift_for_deny_mismatch();           // 4
    test_default_policy_drift_detected();            // 5
    test_drift_score_zero_when_no_observations();    // 6
    test_drift_score_percentage();                   // 7
    test_drift_score_capped_at_100();                // 8
    test_remediation_hint_for_deny_drift();          // 9
    test_remediation_hint_for_review_drift();        // 10
    test_remediation_hint_for_allow_drift();         // 11
    test_multiple_findings_generate_multiple_hints();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
