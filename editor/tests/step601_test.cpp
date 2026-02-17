// Step 601: Compliance Evidence Bundle (12 tests)

#include "ComplianceEvidenceBundle.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ComplianceEvidenceItem item(const std::string& id,
                                   const std::string& control) {
    return {id, control, "/artifacts/" + id + ".json", "summary"};
}

void test_add_evidence_success() {
    TEST(add_evidence_success);
    ComplianceEvidenceBundle b;
    std::string error;
    CHECK(b.addEvidence(item("e1", "ctrl-a"), &error), "add should succeed");
    CHECK(b.artifactCount() == 1, "artifact count mismatch");
    PASS();
}

void test_add_evidence_rejects_missing_item_id() {
    TEST(add_evidence_rejects_missing_item_id);
    ComplianceEvidenceBundle b;
    std::string error;
    auto e = item("", "ctrl-a");
    CHECK(!b.addEvidence(e, &error), "add should fail");
    CHECK(error == "item_id_missing", "wrong error");
    PASS();
}

void test_add_evidence_rejects_missing_control_id() {
    TEST(add_evidence_rejects_missing_control_id);
    ComplianceEvidenceBundle b;
    std::string error;
    auto e = item("e1", "");
    CHECK(!b.addEvidence(e, &error), "add should fail");
    CHECK(error == "control_id_missing", "wrong error");
    PASS();
}

void test_add_evidence_rejects_missing_artifact_path() {
    TEST(add_evidence_rejects_missing_artifact_path);
    ComplianceEvidenceBundle b;
    std::string error;
    auto e = item("e1", "ctrl-a");
    e.artifactPath.clear();
    CHECK(!b.addEvidence(e, &error), "add should fail");
    CHECK(error == "artifact_path_missing", "wrong error");
    PASS();
}

void test_add_evidence_rejects_missing_summary() {
    TEST(add_evidence_rejects_missing_summary);
    ComplianceEvidenceBundle b;
    std::string error;
    auto e = item("e1", "ctrl-a");
    e.summary.clear();
    CHECK(!b.addEvidence(e, &error), "add should fail");
    CHECK(error == "summary_missing", "wrong error");
    PASS();
}

void test_add_evidence_rejects_duplicate() {
    TEST(add_evidence_rejects_duplicate);
    ComplianceEvidenceBundle b;
    std::string error;
    CHECK(b.addEvidence(item("e1", "ctrl-a"), &error), "first add failed");
    CHECK(!b.addEvidence(item("e1", "ctrl-b"), &error), "duplicate should fail");
    CHECK(error == "item_duplicate", "wrong error");
    PASS();
}

void test_by_control_filters_results() {
    TEST(by_control_filters_results);
    ComplianceEvidenceBundle b;
    std::string error;
    CHECK(b.addEvidence(item("e1", "ctrl-a"), &error), "add e1 failed");
    CHECK(b.addEvidence(item("e2", "ctrl-b"), &error), "add e2 failed");
    CHECK(b.byControl("ctrl-a").size() == 1, "filter size mismatch");
    PASS();
}

void test_by_control_empty_returns_all() {
    TEST(by_control_empty_returns_all);
    ComplianceEvidenceBundle b;
    std::string error;
    CHECK(b.addEvidence(item("e1", "ctrl-a"), &error), "add e1 failed");
    CHECK(b.addEvidence(item("e2", "ctrl-b"), &error), "add e2 failed");
    CHECK(b.byControl("").size() == 2, "empty filter should return all");
    PASS();
}

void test_control_coverage_count_unique_controls() {
    TEST(control_coverage_count_unique_controls);
    ComplianceEvidenceBundle b;
    std::string error;
    CHECK(b.addEvidence(item("e1", "ctrl-a"), &error), "add e1 failed");
    CHECK(b.addEvidence(item("e2", "ctrl-a"), &error), "add e2 failed");
    CHECK(b.addEvidence(item("e3", "ctrl-b"), &error), "add e3 failed");
    CHECK(b.controlCoverageCount() == 2, "coverage count mismatch");
    PASS();
}

void test_artifact_count_matches_entries() {
    TEST(artifact_count_matches_entries);
    ComplianceEvidenceBundle b;
    std::string error;
    CHECK(b.addEvidence(item("e1", "ctrl-a"), &error), "add e1 failed");
    CHECK(b.addEvidence(item("e2", "ctrl-b"), &error), "add e2 failed");
    CHECK(b.artifactCount() == 2, "artifact count mismatch");
    PASS();
}

void test_empty_bundle_counts_zero() {
    TEST(empty_bundle_counts_zero);
    ComplianceEvidenceBundle b;
    CHECK(b.artifactCount() == 0, "empty artifact count should be zero");
    CHECK(b.controlCoverageCount() == 0, "empty control count should be zero");
    PASS();
}

void test_by_control_unknown_returns_empty() {
    TEST(by_control_unknown_returns_empty);
    ComplianceEvidenceBundle b;
    std::string error;
    CHECK(b.addEvidence(item("e1", "ctrl-a"), &error), "add e1 failed");
    CHECK(b.byControl("missing").empty(), "unknown control should return empty");
    PASS();
}

int main() {
    std::cout << "Step 601: Compliance Evidence Bundle\n";

    test_add_evidence_success();                      // 1
    test_add_evidence_rejects_missing_item_id();     // 2
    test_add_evidence_rejects_missing_control_id();  // 3
    test_add_evidence_rejects_missing_artifact_path();// 4
    test_add_evidence_rejects_missing_summary();     // 5
    test_add_evidence_rejects_duplicate();           // 6
    test_by_control_filters_results();               // 7
    test_by_control_empty_returns_all();             // 8
    test_control_coverage_count_unique_controls();   // 9
    test_artifact_count_matches_entries();           // 10
    test_empty_bundle_counts_zero();                 // 11
    test_by_control_unknown_returns_empty();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
