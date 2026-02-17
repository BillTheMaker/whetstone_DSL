// Step 562: Workflow-Aware Debug Hooks (12 tests)

#include "WorkflowAwareDebugHooks.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_attach_binding_succeeds() {
    TEST(attach_binding_succeeds);
    WorkflowAwareDebugHooks h;
    CHECK(h.attach("wi-1", "ds-1", {"d1"}), "attach should succeed");
    CHECK(h.isAttached("wi-1"), "binding should exist");
    PASS();
}

void test_attach_rejects_duplicate_workflow_item() {
    TEST(attach_rejects_duplicate_workflow_item);
    WorkflowAwareDebugHooks h;
    CHECK(h.attach("wi-1", "ds-1", {"d1"}), "first attach should succeed");
    CHECK(!h.attach("wi-1", "ds-2", {"d2"}), "duplicate workflow item should fail");
    PASS();
}

void test_attach_rejects_missing_ids() {
    TEST(attach_rejects_missing_ids);
    WorkflowAwareDebugHooks h;
    CHECK(!h.attach("", "ds-1", {}), "missing workflow id should fail");
    CHECK(!h.attach("wi-1", "", {}), "missing session id should fail");
    PASS();
}

void test_detach_removes_binding() {
    TEST(detach_removes_binding);
    WorkflowAwareDebugHooks h;
    CHECK(h.attach("wi-1", "ds-1", {"d1"}), "attach should succeed");
    CHECK(h.detach("wi-1"), "detach should succeed");
    CHECK(!h.isAttached("wi-1"), "binding should be removed");
    PASS();
}

void test_detach_fails_for_unknown_item() {
    TEST(detach_fails_for_unknown_item);
    WorkflowAwareDebugHooks h;
    CHECK(!h.detach("missing"), "detach unknown should fail");
    PASS();
}

void test_set_active_toggles_binding_state() {
    TEST(set_active_toggles_binding_state);
    WorkflowAwareDebugHooks h;
    CHECK(h.attach("wi-1", "ds-1", {"d1"}), "attach should succeed");
    CHECK(h.setActive("wi-1", false), "set inactive should succeed");
    CHECK(h.activeBindings().empty(), "no active bindings expected");
    CHECK(h.setActive("wi-1", true), "set active should succeed");
    CHECK(h.activeBindings().size() == 1, "one active binding expected");
    PASS();
}

void test_set_active_fails_for_unknown_item() {
    TEST(set_active_fails_for_unknown_item);
    WorkflowAwareDebugHooks h;
    CHECK(!h.setActive("missing", true), "set active unknown should fail");
    PASS();
}

void test_add_diagnostic_appends_unique_diagnostic() {
    TEST(add_diagnostic_appends_unique_diagnostic);
    WorkflowAwareDebugHooks h;
    CHECK(h.attach("wi-1", "ds-1", {"d1"}), "attach should succeed");
    CHECK(h.addDiagnostic("wi-1", "d2"), "add diagnostic should succeed");
    auto d = h.diagnosticsFor("wi-1");
    CHECK(d.size() == 2, "two diagnostics expected");
    PASS();
}

void test_add_diagnostic_rejects_duplicate() {
    TEST(add_diagnostic_rejects_duplicate);
    WorkflowAwareDebugHooks h;
    CHECK(h.attach("wi-1", "ds-1", {"d1"}), "attach should succeed");
    CHECK(!h.addDiagnostic("wi-1", "d1"), "duplicate diagnostic should fail");
    PASS();
}

void test_add_diagnostic_fails_for_unknown_binding() {
    TEST(add_diagnostic_fails_for_unknown_binding);
    WorkflowAwareDebugHooks h;
    CHECK(!h.addDiagnostic("missing", "d1"), "unknown binding should fail");
    PASS();
}

void test_active_bindings_returns_only_active() {
    TEST(active_bindings_returns_only_active);
    WorkflowAwareDebugHooks h;
    CHECK(h.attach("wi-1", "ds-1", {"d1"}), "attach wi-1");
    CHECK(h.attach("wi-2", "ds-2", {"d2"}), "attach wi-2");
    CHECK(h.setActive("wi-2", false), "deactivate wi-2");
    auto active = h.activeBindings();
    CHECK(active.size() == 1, "only one active expected");
    CHECK(active[0].workflowItemId == "wi-1", "active item mismatch");
    PASS();
}

void test_diagnostics_for_unknown_item_empty() {
    TEST(diagnostics_for_unknown_item_empty);
    WorkflowAwareDebugHooks h;
    CHECK(h.diagnosticsFor("missing").empty(), "unknown diagnostics should be empty");
    PASS();
}

int main() {
    std::cout << "Step 562: Workflow-Aware Debug Hooks\n";

    test_attach_binding_succeeds();                    // 1
    test_attach_rejects_duplicate_workflow_item();    // 2
    test_attach_rejects_missing_ids();                // 3
    test_detach_removes_binding();                    // 4
    test_detach_fails_for_unknown_item();             // 5
    test_set_active_toggles_binding_state();          // 6
    test_set_active_fails_for_unknown_item();         // 7
    test_add_diagnostic_appends_unique_diagnostic();  // 8
    test_add_diagnostic_rejects_duplicate();          // 9
    test_add_diagnostic_fails_for_unknown_binding();  // 10
    test_active_bindings_returns_only_active();       // 11
    test_diagnostics_for_unknown_item_empty();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
