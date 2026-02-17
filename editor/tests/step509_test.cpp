// Step 509: Docking Reliability Audit + Deterministic Layout Rules (12 tests)

#include "DockingReliability.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static PanelManager makeBaseline() {
    PanelManager pm;
    pm.registerDefaults();
    pm.focusPanel("code-editor");
    return pm;
}

void test_stable_default_state_has_no_issues() {
    TEST(stable_default_state_has_no_issues);
    auto result = DockingReliability::auditAndRepair(makeBaseline());
    CHECK(result.stable, "expected stable baseline state");
    CHECK(result.issues.empty(), "expected no issues");
    PASS();
}

void test_invalid_layout_ratios_are_clamped() {
    TEST(invalid_layout_ratios_are_clamped);
    auto pm = makeBaseline();
    pm.layout().leftWidth = -10.0f;
    pm.layout().rightWidth = 95.0f;
    pm.layout().bottomHeight = 2.0f;
    auto result = DockingReliability::auditAndRepair(pm);
    CHECK(result.state.layout().leftWidth >= 5.0f, "left not clamped");
    CHECK(result.state.layout().rightWidth <= 45.0f, "right not clamped");
    CHECK(result.state.layout().bottomHeight >= 10.0f, "bottom not clamped");
    PASS();
}

void test_hidden_editor_is_forced_visible() {
    TEST(hidden_editor_is_forced_visible);
    auto pm = makeBaseline();
    pm.setVisible("code-editor", false);
    auto result = DockingReliability::auditAndRepair(pm);
    CHECK(result.state.isVisible("code-editor"), "editor should be visible");
    PASS();
}

void test_editor_dock_drift_is_repaired() {
    TEST(editor_dock_drift_is_repaired);
    auto pm = makeBaseline();
    pm.movePanelToDock("code-editor", DockPosition::Left);
    auto result = DockingReliability::auditAndRepair(pm);
    auto p = result.state.getPanel("code-editor");
    CHECK(p.currentDock == p.defaultDock, "editor dock should reset to default");
    PASS();
}

void test_empty_panel_set_restores_defaults() {
    TEST(empty_panel_set_restores_defaults);
    PanelManager pm;
    auto result = DockingReliability::auditAndRepair(pm);
    CHECK(result.state.panelCount() >= 8, "expected default panels restored");
    CHECK(result.state.hasPanel("code-editor"), "expected code-editor panel");
    PASS();
}

void test_deterministic_reset_normalizes_drift() {
    TEST(deterministic_reset_normalizes_drift);
    auto pm = makeBaseline();
    pm.movePanelToDock("file-tree", DockPosition::Right);
    pm.movePanelToDock("code-editor", DockPosition::Bottom);
    pm.setVisible("diagnostics", false);
    auto result = DockingReliability::auditAndRepair(pm, true);
    CHECK(result.state.getPanel("file-tree").currentDock ==
          result.state.getPanel("file-tree").defaultDock,
          "file-tree should match default dock");
    CHECK(result.state.getPanel("code-editor").currentDock ==
          result.state.getPanel("code-editor").defaultDock,
          "editor should match default dock");
    PASS();
}

void test_deterministic_reset_has_stable_fingerprint() {
    TEST(deterministic_reset_has_stable_fingerprint);
    auto pm = makeBaseline();
    pm.movePanelToDock("workflow", DockPosition::Left);
    auto a = DockingReliability::auditAndRepair(pm, true);
    auto b = DockingReliability::auditAndRepair(a.state, true);
    CHECK(a.fingerprintAfter == b.fingerprintAfter, "fingerprint should be deterministic");
    PASS();
}

void test_invalid_focus_is_repaired_to_editor() {
    TEST(invalid_focus_is_repaired_to_editor);
    auto pm = makeBaseline();
    pm.focusPanel("ghost");
    auto result = DockingReliability::auditAndRepair(pm);
    CHECK(result.state.getFocusedPanel() == "code-editor", "focus should be repaired");
    PASS();
}

void test_missing_editor_panel_is_inserted() {
    TEST(missing_editor_panel_is_inserted);
    auto pm = makeBaseline();
    auto j = pm.toJson();
    nlohmann::json kept = nlohmann::json::array();
    for (const auto& p : j["panels"]) {
        if (p.value("id", "") != "code-editor") kept.push_back(p);
    }
    j["panels"] = kept;
    auto broken = PanelManager::fromJson(j);
    auto result = DockingReliability::auditAndRepair(broken);
    CHECK(result.state.hasPanel("code-editor"), "editor should be reinserted");
    PASS();
}

void test_no_visible_center_panel_is_fixed() {
    TEST(no_visible_center_panel_is_fixed);
    auto pm = makeBaseline();
    pm.setVisible("code-editor", false);
    auto result = DockingReliability::auditAndRepair(pm);
    bool hasCenter = false;
    for (const auto& p : result.state.getVisiblePanels()) {
        if (p.currentDock == DockPosition::CenterTop ||
            p.currentDock == DockPosition::CenterBottom) {
            hasCenter = true;
            break;
        }
    }
    CHECK(hasCenter, "at least one center panel should be visible");
    PASS();
}

void test_reconcile_from_session_keeps_known_panels() {
    TEST(reconcile_from_session_keeps_known_panels);
    auto saved = makeBaseline();
    saved.movePanelToDock("Explorer", DockPosition::Right); // unknown in defaults, ignored by merge
    saved.movePanelToDock("Panel", DockPosition::Left);     // unknown in defaults, ignored by merge
    auto merged = DockingReliability::reconcileFromSession(saved, LayoutPreset::VSCode);
    CHECK(merged.hasPanel("Editor"), "preset panel should exist");
    CHECK(merged.hasPanel("Explorer"), "preset explorer should exist");
    PASS();
}

void test_issue_codes_are_reported_for_recovery_paths() {
    TEST(issue_codes_are_reported_for_recovery_paths);
    PanelManager pm;
    auto result = DockingReliability::auditAndRepair(pm, true);
    bool hasD001 = false;
    bool hasD008 = false;
    for (const auto& i : result.issues) {
        if (i.code == "D001") hasD001 = true;
        if (i.code == "D008") hasD008 = true;
    }
    CHECK(hasD001, "expected empty-session recovery code D001");
    CHECK(hasD008, "expected deterministic reset code D008");
    PASS();
}

int main() {
    std::cout << "Step 509: Docking Reliability Audit + Deterministic Layout Rules\n";

    test_stable_default_state_has_no_issues();                 // 1
    test_invalid_layout_ratios_are_clamped();                  // 2
    test_hidden_editor_is_forced_visible();                    // 3
    test_editor_dock_drift_is_repaired();                      // 4
    test_empty_panel_set_restores_defaults();                  // 5
    test_deterministic_reset_normalizes_drift();               // 6
    test_deterministic_reset_has_stable_fingerprint();         // 7
    test_invalid_focus_is_repaired_to_editor();                // 8
    test_missing_editor_panel_is_inserted();                   // 9
    test_no_visible_center_panel_is_fixed();                   // 10
    test_reconcile_from_session_keeps_known_panels();          // 11
    test_issue_codes_are_reported_for_recovery_paths();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
