#pragma once
// Step 514: Phase 26a Integration
// Integrates Steps 509-513 into a single reliability gate for docking,
// interaction, hierarchy, and accessibility consistency.

#include "DockingReliability.h"
#include "PanelBoundaryReliability.h"
#include "InteractionStateModel.h"
#include "WindowHierarchyModel.h"
#include "AccessibilityBaseline.h"

#include <string>
#include <vector>

struct Phase26aIntegrationResult {
    bool dockingStable = false;
    bool hitTargetsReliable = false;
    bool interactionStatesValid = false;
    bool hierarchyValid = false;
    bool accessibilityBaselineValid = false;
    bool editorFlowSafe = false;
    bool pass = false;
    std::vector<std::string> notes;
};

class Phase26aIntegration {
public:
    static Phase26aIntegrationResult run() {
        Phase26aIntegrationResult out;

        // 509: docking/layout reliability
        PanelManager pm;
        pm.registerDefaults();
        pm.focusPanel("code-editor");
        auto dock = DockingReliability::auditAndRepair(pm, true);
        out.dockingStable = dock.state.hasPanel("code-editor") &&
                            dock.state.isVisible("code-editor") &&
                            dock.state.layout().centerWidth >= 10.0f;
        if (!out.dockingStable) out.notes.push_back("docking-not-stable");

        // 510: split-bar hit targets and resize priority
        std::vector<PanelRect> panels = {
            {"left", 0, 0, 220, 800},
            {"editor", 220, 0, 760, 800},
            {"right", 980, 0, 220, 800}
        };
        std::vector<SplitBar> bars = {
            {"left-editor", true, 220, 6, "left", "editor"},
            {"editor-right", true, 980, 6, "editor", "right"}
        };
        auto hit = PanelBoundaryReliability::resolveClick(221, 100, panels, bars);
        auto norm = PanelBoundaryReliability::normalizeGeometry(panels, bars);
        out.hitTargetsReliable = hit.resizeHit && hit.targetId == "left-editor" &&
                                 norm.bars[0].thickness >= 5.0f;
        if (!out.hitTargetsReliable) out.notes.push_back("hit-target-not-reliable");

        // 511: interaction-state validity
        auto tokens = InteractionStateModel::defaultDarkTokens();
        auto styleFocus = InteractionStateModel::resolveStyle(
            {false, true, false, false, false}, tokens);
        auto stylePressed = InteractionStateModel::resolveStyle(
            {true, true, true, true, false}, tokens);
        out.interactionStatesValid = styleFocus.showFocusRing &&
                                     styleFocus.contrastPassAA &&
                                     stylePressed.state == InteractionState::Pressed;
        if (!out.interactionStatesValid) out.notes.push_back("interaction-state-invalid");

        // 512: hierarchy/layer semantics
        std::vector<SurfaceNode> surfaces = {
            {"editor", SurfaceRole::Work, false, true, 0, {}},
            {"tool", SurfaceRole::Tool, false, true, 0, {}},
            {"overlay", SurfaceRole::Overlay, false, true, 0, {}},
            {"dialog", SurfaceRole::Modal, true, true, 0, {}}
        };
        auto hierarchy = WindowHierarchyModel::validateAndOrder(surfaces);
        out.hierarchyValid = hierarchy.valid;
        if (!out.hierarchyValid) out.notes.push_back("hierarchy-invalid");

        // 513: accessibility baseline
        std::vector<AccessibilityNode> controls = {
            {"menu", true, true, true, 0},
            {"explorer", true, true, true, 1},
            {"editor", true, true, true, 2},
            {"status", true, true, true, 3}
        };
        auto ring = AccessibilityBaseline::resolveFocusRing(true, true, 0.1f, 0.9f);
        auto access = AccessibilityBaseline::validate(controls, ring, 7.0f, 4.0f);
        out.accessibilityBaselineValid = access.pass;
        if (!out.accessibilityBaselineValid) out.notes.push_back("accessibility-invalid");

        // Combined integration: editor flow should remain safe after all checks.
        out.editorFlowSafe = out.dockingStable && out.hitTargetsReliable &&
                             out.interactionStatesValid && out.hierarchyValid &&
                             out.accessibilityBaselineValid;

        out.pass = out.editorFlowSafe;
        if (out.pass) out.notes.push_back("phase-26a-integration-pass");
        return out;
    }
};
