// Step 598: Phase 34a Integration (8 tests)

#include "Phase34aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Phase34aResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

void test_full_phase34a_cycle_passes() {
    TEST(full_phase34a_cycle_passes);
    auto r = Phase34aIntegration::run();
    CHECK(r.phase34aPass, "phase should pass");
    CHECK(r.guardrailCatalogReady, "catalog should be ready");
    CHECK(r.escalationPlannerReady, "planner should be ready");
    CHECK(r.auditLedgerReady, "ledger should be ready");
    CHECK(r.driftMonitorReady, "drift monitor should be ready");
    CHECK(hasNote(r, "phase34a:policy_governance_cycle_ready"), "success note missing");
    PASS();
}

void test_guardrail_catalog_signal_true_on_pass() {
    TEST(guardrail_catalog_signal_true_on_pass);
    auto r = Phase34aIntegration::run();
    CHECK(r.phase34aPass, "phase should pass");
    CHECK(r.guardrailCatalogReady, "catalog flag should be true");
    PASS();
}

void test_escalation_planner_signal_true_on_pass() {
    TEST(escalation_planner_signal_true_on_pass);
    auto r = Phase34aIntegration::run();
    CHECK(r.phase34aPass, "phase should pass");
    CHECK(r.escalationPlannerReady, "planner flag should be true");
    PASS();
}

void test_audit_ledger_signal_true_on_pass() {
    TEST(audit_ledger_signal_true_on_pass);
    auto r = Phase34aIntegration::run();
    CHECK(r.phase34aPass, "phase should pass");
    CHECK(r.auditLedgerReady, "ledger flag should be true");
    PASS();
}

void test_drift_monitor_signal_true_on_pass() {
    TEST(drift_monitor_signal_true_on_pass);
    auto r = Phase34aIntegration::run();
    CHECK(r.phase34aPass, "phase should pass");
    CHECK(r.driftMonitorReady, "drift flag should be true");
    CHECK(r.driftScore > 0, "drift score should be positive");
    PASS();
}

void test_no_blocked_note_on_success() {
    TEST(no_blocked_note_on_success);
    auto r = Phase34aIntegration::run();
    CHECK(r.phase34aPass, "phase should pass");
    CHECK(!hasNote(r, "phase34a:blocked"), "blocked note should not appear");
    PASS();
}

void test_notes_include_only_success_marker_on_pass() {
    TEST(notes_include_only_success_marker_on_pass);
    auto r = Phase34aIntegration::run();
    CHECK(r.phase34aPass, "phase should pass");
    CHECK(r.notes.size() == 1, "expected one success note");
    PASS();
}

void test_phase_flag_matches_component_conjunction() {
    TEST(phase_flag_matches_component_conjunction);
    auto r = Phase34aIntegration::run();
    const bool conjunction = r.guardrailCatalogReady && r.escalationPlannerReady &&
                             r.auditLedgerReady && r.driftMonitorReady;
    CHECK(r.phase34aPass == conjunction, "phase flag should equal conjunction");
    PASS();
}

int main() {
    std::cout << "Step 598: Phase 34a Integration\n";

    test_full_phase34a_cycle_passes();               // 1
    test_guardrail_catalog_signal_true_on_pass();    // 2
    test_escalation_planner_signal_true_on_pass();   // 3
    test_audit_ledger_signal_true_on_pass();         // 4
    test_drift_monitor_signal_true_on_pass();        // 5
    test_no_blocked_note_on_success();               // 6
    test_notes_include_only_success_marker_on_pass();// 7
    test_phase_flag_matches_component_conjunction(); // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
