// Step 589: Release Readiness Gate Pack (12 tests)

#include "ReleaseReadinessGatePack.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const ReleaseReadinessResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static ReleaseGateSignals allGreen() {
    ReleaseGateSignals s;
    s.uxChecks = true;
    s.runtimeChecks = true;
    s.regressionChecks = true;
    s.telemetryChecks = true;
    s.documentationChecks = true;
    return s;
}

void test_all_green_ready() {
    TEST(all_green_ready);
    auto r = ReleaseReadinessGatePack::evaluate(allGreen());
    CHECK(r.ready, "release should be ready");
    PASS();
}

void test_missing_ux_blocks_release() {
    TEST(missing_ux_blocks_release);
    auto s = allGreen();
    s.uxChecks = false;
    auto r = ReleaseReadinessGatePack::evaluate(s);
    CHECK(!r.ready, "release should be blocked");
    CHECK(!r.blockedGates.empty() && r.blockedGates[0] == "ux", "ux should be blocked");
    PASS();
}

void test_missing_runtime_blocks_release() {
    TEST(missing_runtime_blocks_release);
    auto s = allGreen();
    s.runtimeChecks = false;
    auto r = ReleaseReadinessGatePack::evaluate(s);
    CHECK(!r.ready, "release should be blocked");
    CHECK(hasNote(r, "blocked:runtime"), "runtime note expected");
    PASS();
}

void test_missing_regression_blocks_release() {
    TEST(missing_regression_blocks_release);
    auto s = allGreen();
    s.regressionChecks = false;
    auto r = ReleaseReadinessGatePack::evaluate(s);
    CHECK(!r.ready, "release should be blocked");
    CHECK(hasNote(r, "blocked:regression"), "regression note expected");
    PASS();
}

void test_missing_telemetry_blocks_release() {
    TEST(missing_telemetry_blocks_release);
    auto s = allGreen();
    s.telemetryChecks = false;
    auto r = ReleaseReadinessGatePack::evaluate(s);
    CHECK(!r.ready, "release should be blocked");
    CHECK(hasNote(r, "blocked:telemetry"), "telemetry note expected");
    PASS();
}

void test_missing_documentation_blocks_release() {
    TEST(missing_documentation_blocks_release);
    auto s = allGreen();
    s.documentationChecks = false;
    auto r = ReleaseReadinessGatePack::evaluate(s);
    CHECK(!r.ready, "release should be blocked");
    CHECK(hasNote(r, "blocked:documentation"), "documentation note expected");
    PASS();
}

void test_multiple_missing_gates_accumulate() {
    TEST(multiple_missing_gates_accumulate);
    ReleaseGateSignals s;
    auto r = ReleaseReadinessGatePack::evaluate(s);
    CHECK(r.blockedGates.size() == 5, "all gates should be blocked");
    PASS();
}

void test_release_go_notes_emitted_on_ready() {
    TEST(release_go_notes_emitted_on_ready);
    auto r = ReleaseReadinessGatePack::evaluate(allGreen());
    CHECK(hasNote(r, "release:go"), "go note expected");
    CHECK(hasNote(r, "release:gates_all_green"), "green note expected");
    PASS();
}

void test_release_no_go_note_on_failure() {
    TEST(release_no_go_note_on_failure);
    ReleaseGateSignals s;
    auto r = ReleaseReadinessGatePack::evaluate(s);
    CHECK(hasNote(r, "release:no_go"), "no-go note expected");
    PASS();
}

void test_hotfix_allowed_with_runtime_and_regression() {
    TEST(hotfix_allowed_with_runtime_and_regression);
    ReleaseGateSignals s;
    s.runtimeChecks = true;
    s.regressionChecks = true;
    CHECK(ReleaseReadinessGatePack::canShipHotfix(s), "hotfix should be allowed");
    PASS();
}

void test_hotfix_blocked_without_runtime_or_regression() {
    TEST(hotfix_blocked_without_runtime_or_regression);
    ReleaseGateSignals s;
    s.runtimeChecks = true;
    s.regressionChecks = false;
    CHECK(!ReleaseReadinessGatePack::canShipHotfix(s), "hotfix should be blocked");
    PASS();
}

void test_readiness_score_weights_applied() {
    TEST(readiness_score_weights_applied);
    auto s = allGreen();
    CHECK(ReleaseReadinessGatePack::readinessScore(s) == 100, "all-green score should be 100");
    s.telemetryChecks = false;
    CHECK(ReleaseReadinessGatePack::readinessScore(s) == 85, "score should drop by telemetry weight");
    PASS();
}

int main() {
    std::cout << "Step 589: Release Readiness Gate Pack\n";

    test_all_green_ready();                          // 1
    test_missing_ux_blocks_release();               // 2
    test_missing_runtime_blocks_release();          // 3
    test_missing_regression_blocks_release();       // 4
    test_missing_telemetry_blocks_release();        // 5
    test_missing_documentation_blocks_release();    // 6
    test_multiple_missing_gates_accumulate();       // 7
    test_release_go_notes_emitted_on_ready();       // 8
    test_release_no_go_note_on_failure();           // 9
    test_hotfix_allowed_with_runtime_and_regression();// 10
    test_hotfix_blocked_without_runtime_or_regression();// 11
    test_readiness_score_weights_applied();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
