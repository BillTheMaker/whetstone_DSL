#pragma once
// Step 509: Docking Reliability Audit + Deterministic Layout Rules
// Headless guardrails for panel drift/visibility issues and deterministic
// startup/session-restore layout behavior.

#include "PanelManager.h"
#include "LayoutManager.h"

#include <string>
#include <vector>
#include <sstream>
#include <cmath>

struct DockingIssue {
    std::string code;
    std::string message;
    std::string panelId;
    std::string severity = "warning";
};

struct DockingAuditResult {
    bool stable = true;
    bool repaired = false;
    PanelManager state;
    std::vector<DockingIssue> issues;
    std::string fingerprintBefore;
    std::string fingerprintAfter;
};

class DockingReliability {
public:
    static DockingAuditResult auditAndRepair(const PanelManager& input,
                                             bool deterministicReset = false) {
        DockingAuditResult out;
        out.state = input;
        out.fingerprintBefore = layoutFingerprint(input);

        ensurePanelsPresent(out.state, out);
        clampLayout(out.state, out);
        repairCriticalVisibility(out.state, out);
        repairEditorDockDrift(out.state, out);
        repairFocusState(out.state, out);

        if (deterministicReset) {
            applyDeterministicReset(out.state, out);
        }

        out.fingerprintAfter = layoutFingerprint(out.state);
        out.stable = out.issues.empty();
        out.repaired = out.fingerprintBefore != out.fingerprintAfter;
        return out;
    }

    static PanelManager deterministicStateFromPreset(LayoutPreset preset) {
        LayoutManager lm;
        lm.setPreset(preset);
        const LayoutConfig& cfg = lm.getConfig();

        PanelManager state;
        int order = 0;
        for (const auto& panel : cfg.panels) {
            state.registerPanel(panel.id, panel.id,
                                toDockPosition(panel.direction),
                                "", "layout", order++);
            state.setVisible(panel.id, panel.visible);
        }

        state.layout().leftWidth = ratioOr(state, DockDirection::Left, 20.0f);
        state.layout().rightWidth = ratioOr(state, DockDirection::Right, 20.0f);
        state.layout().bottomHeight = ratioOr(state, DockDirection::Bottom, 25.0f);
        state.layout().centerWidth =
            100.0f - state.layout().leftWidth - state.layout().rightWidth;

        if (state.hasPanel("Editor")) state.focusPanel("Editor");
        return auditAndRepair(state, true).state;
    }

    static PanelManager reconcileFromSession(const PanelManager& saved,
                                             LayoutPreset preset) {
        PanelManager merged = deterministicStateFromPreset(preset);
        for (const auto& p : saved.getPanelList()) {
            if (!merged.hasPanel(p.id)) continue;
            merged.setVisible(p.id, p.isVisible);
            merged.movePanelToDock(p.id, p.currentDock);
        }
        if (!saved.getFocusedPanel().empty() && merged.hasPanel(saved.getFocusedPanel())) {
            merged.focusPanel(saved.getFocusedPanel());
        }
        return auditAndRepair(merged, false).state;
    }

    static std::string layoutFingerprint(const PanelManager& state) {
        std::ostringstream oss;
        auto panels = state.getPanelList();
        oss << "L:" << rounded(state.layout().leftWidth)
            << ",R:" << rounded(state.layout().rightWidth)
            << ",B:" << rounded(state.layout().bottomHeight)
            << ",C:" << rounded(state.layout().centerWidth)
            << "|";
        for (const auto& p : panels) {
            oss << p.id << ":" << dockPositionToString(p.currentDock)
                << ":" << (p.isVisible ? "1" : "0")
                << ":" << p.order << ";";
        }
        oss << "F:" << state.getFocusedPanel();
        return oss.str();
    }

private:
    static int rounded(float v) {
        return static_cast<int>(std::round(v));
    }

    static bool isCriticalPanelId(const std::string& id) {
        return id == "code-editor" || id == "Editor";
    }

    static DockPosition toDockPosition(DockDirection d) {
        switch (d) {
            case DockDirection::Left: return DockPosition::Left;
            case DockDirection::Right: return DockPosition::Right;
            case DockDirection::Bottom: return DockPosition::Bottom;
            case DockDirection::Top: return DockPosition::CenterTop;
            case DockDirection::Center: return DockPosition::CenterTop;
        }
        return DockPosition::CenterTop;
    }

    static float ratioOr(const PanelManager& state,
                         DockDirection dir,
                         float fallback) {
        // Reconstruct from panel width hints if present in the source config
        // is not available. Use conservative defaults for deterministic startup.
        (void)state;
        (void)dir;
        return fallback;
    }

    static void addIssue(DockingAuditResult& out,
                         const std::string& code,
                         const std::string& message,
                         const std::string& panelId = "",
                         const std::string& severity = "warning") {
        DockingIssue i;
        i.code = code;
        i.message = message;
        i.panelId = panelId;
        i.severity = severity;
        out.issues.push_back(i);
    }

    static void ensurePanelsPresent(PanelManager& state,
                                    DockingAuditResult& out) {
        if (state.panelCount() == 0) {
            state.registerDefaults();
            addIssue(out, "D001", "Session contained no panels; restored default panel set", "", "error");
            return;
        }

        if (!state.hasPanel("code-editor") && !state.hasPanel("Editor")) {
            state.registerPanel("code-editor", "Code Editor", DockPosition::CenterTop, "", "editor", 1);
            addIssue(out, "D002", "Critical editor panel missing; inserted fallback code-editor", "code-editor", "error");
        }
    }

    static void clampLayout(PanelManager& state,
                            DockingAuditResult& out) {
        bool touched = false;
        auto& l = state.layout();

        if (l.leftWidth < 5.0f) {
            l.leftWidth = 5.0f;
            touched = true;
        }
        if (l.leftWidth > 45.0f) {
            l.leftWidth = 45.0f;
            touched = true;
        }
        if (l.rightWidth < 5.0f) {
            l.rightWidth = 5.0f;
            touched = true;
        }
        if (l.rightWidth > 45.0f) {
            l.rightWidth = 45.0f;
            touched = true;
        }
        if (l.bottomHeight < 10.0f) {
            l.bottomHeight = 10.0f;
            touched = true;
        }
        if (l.bottomHeight > 60.0f) {
            l.bottomHeight = 60.0f;
            touched = true;
        }

        l.centerWidth = 100.0f - l.leftWidth - l.rightWidth;
        if (l.centerWidth < 10.0f) {
            // Rebalance sides deterministically to guarantee editor viewport.
            l.leftWidth = 20.0f;
            l.rightWidth = 20.0f;
            l.centerWidth = 60.0f;
            touched = true;
        }

        if (touched) {
            addIssue(out, "D003", "Layout ratios out of bounds; clamped to deterministic safe ranges");
        }
    }

    static void repairCriticalVisibility(PanelManager& state,
                                         DockingAuditResult& out) {
        bool hasVisibleCenter = false;
        for (const auto& p : state.getVisiblePanels()) {
            if (p.currentDock == DockPosition::CenterTop ||
                p.currentDock == DockPosition::CenterBottom) {
                hasVisibleCenter = true;
                break;
            }
        }

        if (!hasVisibleCenter) {
            if (state.hasPanel("code-editor")) {
                state.setVisible("code-editor", true);
                addIssue(out, "D004", "No visible center panel; forced code-editor visible", "code-editor", "error");
            } else if (state.hasPanel("Editor")) {
                state.setVisible("Editor", true);
                addIssue(out, "D004", "No visible center panel; forced Editor visible", "Editor", "error");
            }
        }

        for (const auto& p : state.getPanelList()) {
            if (isCriticalPanelId(p.id) && !p.isVisible) {
                state.setVisible(p.id, true);
                addIssue(out, "D005", "Critical editor panel hidden; forced visible", p.id, "error");
            }
        }
    }

    static void repairEditorDockDrift(PanelManager& state,
                                      DockingAuditResult& out) {
        for (const auto& p : state.getPanelList()) {
            if (!isCriticalPanelId(p.id)) continue;
            if (!(p.currentDock == DockPosition::CenterTop ||
                  p.currentDock == DockPosition::CenterBottom)) {
                state.movePanelToDock(p.id, p.defaultDock);
                addIssue(out, "D006", "Editor panel drifted from center; restored to default dock", p.id);
            }
        }
    }

    static void repairFocusState(PanelManager& state,
                                 DockingAuditResult& out) {
        std::string focused = state.getFocusedPanel();
        if (!focused.empty() && state.hasPanel(focused)) return;

        if (state.hasPanel("code-editor")) {
            state.focusPanel("code-editor");
            addIssue(out, "D007", "Focused panel invalid; restored focus to code-editor");
            return;
        }
        if (state.hasPanel("Editor")) {
            state.focusPanel("Editor");
            addIssue(out, "D007", "Focused panel invalid; restored focus to Editor");
            return;
        }

        auto panels = state.getPanelList();
        if (!panels.empty()) {
            state.focusPanel(panels.front().id);
            addIssue(out, "D007", "Focused panel invalid; restored to first registered panel");
        }
    }

    static void applyDeterministicReset(PanelManager& state,
                                        DockingAuditResult& out) {
        for (const auto& p : state.getPanelList()) {
            state.movePanelToDock(p.id, p.defaultDock);
            if (isCriticalPanelId(p.id)) state.setVisible(p.id, true);
        }
        repairFocusState(state, out);
        addIssue(out, "D008", "Deterministic reset applied", "", "info");
    }
};
