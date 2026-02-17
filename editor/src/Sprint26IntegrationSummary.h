#pragma once
// Step 523: Sprint 26 Integration + Summary

#include "Phase26aIntegration.h"
#include "ThemeTokenSystemV2.h"
#include "SignatureVisualIdentity.h"
#include "ControlLibraryRestyle.h"
#include "IconTypographyHarmonizer.h"
#include "ModifierEdgeNotation.h"
#include "CompletionOverlayUXHardening.h"
#include "NotificationStatusRefresh.h"
#include "CrossPanelConsistencySweep.h"

#include <string>
#include <vector>

struct Sprint26Summary {
    bool phase26aPass = false;
    bool tokenSystemPass = false;
    bool visualIdentityPass = false;
    bool controlsPass = false;
    bool typographyPass = false;
    bool modifierEdgePass = false;
    bool overlayPass = false;
    bool notificationPass = false;
    bool panelConsistencyPass = false;
    bool sprintPass = false;
    std::vector<std::string> notes;
};

class Sprint26IntegrationSummary {
public:
    static Sprint26Summary build() {
        Sprint26Summary s;

        auto p26a = Phase26aIntegration::run();
        s.phase26aPass = p26a.pass;

        auto tokens = ThemeTokenSystemV2::baseBlackStone();
        s.tokenSystemPass = ThemeTokenSystemV2::validateAA(tokens) &&
                            !ThemeTokenSystemV2::hasHardcodedPurple(tokens);

        auto grad = SignatureVisualIdentity::blackSurfaceGradient(true);
        auto link = SignatureVisualIdentity::electricBlueLink(true, 0.2f);
        s.visualIdentityPass = grad.size() == 3 &&
                               SignatureVisualIdentity::isElectricBlue(link);

        auto idle = ControlLibraryRestyle::styleFor(ControlKind::Button, 1.0f, false);
        auto pressed = ControlLibraryRestyle::styleFor(ControlKind::Button, 1.0f, true);
        s.controlsPass = ControlLibraryRestyle::hasClearPressedSignal(idle, pressed) &&
                         ControlLibraryRestyle::validContrast(idle);

        auto ty = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
        auto ic = IconTypographyHarmonizer::iconProfileFor(1.0f, false);
        s.typographyPass = IconTypographyHarmonizer::harmonized(ty, ic);

        auto glyph = ModifierEdgeNotation::buildGlyph('P', true, true, false, false, true);
        s.modifierEdgePass = ModifierEdgeNotation::activeEdgeCount(glyph.edges) == 2 &&
                             glyph.fallbackText == "Ctrl+Shift+P";

        CompletionOverlayState overlay;
        overlay.dismissed = true;
        overlay.lastTriggerToken = "foo";
        auto reopened = CompletionOverlayUXHardening::onTrigger(overlay, "bar", false);
        s.overlayPass = reopened.visible && !reopened.dismissed;

        auto noteStyle = NotificationStatusRefresh::styleFor(NotifyLevel::Critical);
        s.notificationPass = noteStyle.defaultDurationMs == 0.0f &&
                             NotificationStatusRefresh::readable(noteStyle);

        PanelVisualContract base{"base", 12, 6, 1, "v2", "v2"};
        std::vector<PanelVisualContract> panels = {
            {"explorer", 12, 6, 1, "v2", "v2"},
            {"editor", 12, 6, 1, "v2", "v2"},
            {"status", 12, 6, 1, "v2", "v2"}
        };
        auto sweep = CrossPanelConsistencySweep::run(panels, base);
        s.panelConsistencyPass = sweep.pass;

        s.sprintPass = s.phase26aPass && s.tokenSystemPass && s.visualIdentityPass &&
                       s.controlsPass && s.typographyPass && s.modifierEdgePass &&
                       s.overlayPass && s.notificationPass && s.panelConsistencyPass;

        s.notes.push_back("Sprint 26 integration summary generated");
        s.notes.push_back(s.sprintPass
            ? "Sprint 26 complete: UX foundations, visual language, and hardening gates pass"
            : "Sprint 26 incomplete: one or more integration gates failed");
        return s;
    }
};
