// Step 603: Compliance Operational Readiness (12 tests)

#include "ComplianceOperationalReadiness.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ComplianceReadinessSnapshot healthySnapshot() {
    return {4, 6, 3, 0, 2, 0};
}

void test_validate_input_success() {
    TEST(validate_input_success);
    ComplianceReadinessSnapshot s = healthySnapshot();
    std::string error;
    CHECK(ComplianceOperationalReadiness::validateInput(s, &error), "validation should succeed");
    CHECK(error.empty(), "error should be empty");
    PASS();
}

void test_validate_input_rejects_negative_controls_covered() {
    TEST(validate_input_rejects_negative_controls_covered);
    ComplianceReadinessSnapshot s = healthySnapshot();
    s.controlsCovered = -1;
    std::string error;
    CHECK(!ComplianceOperationalReadiness::validateInput(s, &error), "validation should fail");
    CHECK(error == "controls_covered_invalid", "wrong error");
    PASS();
}

void test_validate_input_rejects_negative_artifact_count() {
    TEST(validate_input_rejects_negative_artifact_count);
    ComplianceReadinessSnapshot s = healthySnapshot();
    s.artifactCount = -1;
    std::string error;
    CHECK(!ComplianceOperationalReadiness::validateInput(s, &error), "validation should fail");
    CHECK(error == "artifact_count_invalid", "wrong error");
    PASS();
}

void test_validate_input_rejects_negative_signed_attestations() {
    TEST(validate_input_rejects_negative_signed_attestations);
    ComplianceReadinessSnapshot s = healthySnapshot();
    s.signedAttestations = -1;
    std::string error;
    CHECK(!ComplianceOperationalReadiness::validateInput(s, &error), "validation should fail");
    CHECK(error == "signed_attestations_invalid", "wrong error");
    PASS();
}

void test_validate_input_rejects_negative_expiring_soon() {
    TEST(validate_input_rejects_negative_expiring_soon);
    ComplianceReadinessSnapshot s = healthySnapshot();
    s.expiringSoon = -1;
    std::string error;
    CHECK(!ComplianceOperationalReadiness::validateInput(s, &error), "validation should fail");
    CHECK(error == "expiring_soon_invalid", "wrong error");
    PASS();
}

void test_validate_input_rejects_negative_open_runbooks() {
    TEST(validate_input_rejects_negative_open_runbooks);
    ComplianceReadinessSnapshot s = healthySnapshot();
    s.openRunbooks = -1;
    std::string error;
    CHECK(!ComplianceOperationalReadiness::validateInput(s, &error), "validation should fail");
    CHECK(error == "open_runbooks_invalid", "wrong error");
    PASS();
}

void test_readiness_score_increases_with_positive_signals() {
    TEST(readiness_score_increases_with_positive_signals);
    ComplianceReadinessSnapshot low = {1, 1, 0, 0, 0, 0};
    ComplianceReadinessSnapshot high = {5, 8, 4, 0, 2, 0};
    CHECK(ComplianceOperationalReadiness::readinessScore(high) >
          ComplianceOperationalReadiness::readinessScore(low), "high score should be greater");
    PASS();
}

void test_readiness_score_penalizes_risk_signals() {
    TEST(readiness_score_penalizes_risk_signals);
    ComplianceReadinessSnapshot clean = healthySnapshot();
    ComplianceReadinessSnapshot risky = healthySnapshot();
    risky.expiringSoon = 3;
    risky.openRunbooks = 2;
    CHECK(ComplianceOperationalReadiness::readinessScore(risky) <
          ComplianceOperationalReadiness::readinessScore(clean), "risky score should be lower");
    PASS();
}

void test_readiness_score_clamps_to_zero() {
    TEST(readiness_score_clamps_to_zero);
    ComplianceReadinessSnapshot s = {0, 0, 0, 9, 0, 9};
    CHECK(ComplianceOperationalReadiness::readinessScore(s) == 0, "score should clamp to zero");
    PASS();
}

void test_readiness_score_clamps_to_hundred() {
    TEST(readiness_score_clamps_to_hundred);
    ComplianceReadinessSnapshot s = {20, 20, 20, 0, 10, 0};
    CHECK(ComplianceOperationalReadiness::readinessScore(s) == 100, "score should clamp to hundred");
    PASS();
}

void test_blocking_findings_reports_expected_reasons() {
    TEST(blocking_findings_reports_expected_reasons);
    ComplianceReadinessSnapshot s = {0, 0, 0, 1, 0, 1};
    const auto findings = ComplianceOperationalReadiness::blockingFindings(s);
    CHECK(findings.size() == 5, "finding count mismatch");
    PASS();
}

void test_release_eligibility_requires_no_blockers() {
    TEST(release_eligibility_requires_no_blockers);
    ComplianceReadinessSnapshot healthy = healthySnapshot();
    ComplianceReadinessSnapshot blocked = healthySnapshot();
    blocked.expiringSoon = 1;
    CHECK(ComplianceOperationalReadiness::releaseEligible(healthy), "healthy snapshot should be eligible");
    CHECK(!ComplianceOperationalReadiness::releaseEligible(blocked), "blocked snapshot should be ineligible");
    PASS();
}

int main() {
    std::cout << "Step 603: Compliance Operational Readiness\n";

    test_validate_input_success();                           // 1
    test_validate_input_rejects_negative_controls_covered();// 2
    test_validate_input_rejects_negative_artifact_count();  // 3
    test_validate_input_rejects_negative_signed_attestations();// 4
    test_validate_input_rejects_negative_expiring_soon();   // 5
    test_validate_input_rejects_negative_open_runbooks();   // 6
    test_readiness_score_increases_with_positive_signals(); // 7
    test_readiness_score_penalizes_risk_signals();          // 8
    test_readiness_score_clamps_to_zero();                  // 9
    test_readiness_score_clamps_to_hundred();               // 10
    test_blocking_findings_reports_expected_reasons();      // 11
    test_release_eligibility_requires_no_blockers();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
