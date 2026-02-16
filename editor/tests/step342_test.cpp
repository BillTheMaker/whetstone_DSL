// Step 342: DockingLayout + PanelManager Infrastructure (12 tests)
// Tests panel registration, docking layout, visibility, focus, JSON persistence

#include <cassert>
#include <iostream>
#include <string>
#include "PanelManager.h"

int main() {
    int passed = 0;

    // Test 1: Register 8 default panels
    {
        PanelManager pm;
        pm.registerDefaults();
        assert(pm.panelCount() == 8);
        std::cout << "Test 1 PASSED: Register 8 default panels\n";
        passed++;
    }

    // Test 2: All default panels visible by default
    {
        PanelManager pm;
        pm.registerDefaults();
        auto visible = pm.getVisiblePanels();
        assert(visible.size() == 8);
        std::cout << "Test 2 PASSED: All default panels visible\n";
        passed++;
    }

    // Test 3: Panels in correct dock positions
    {
        PanelManager pm;
        pm.registerDefaults();
        auto left = pm.getPanelsByDock(DockPosition::Left);
        auto right = pm.getPanelsByDock(DockPosition::Right);
        auto bottom = pm.getPanelsByDock(DockPosition::Bottom);
        auto center = pm.getPanelsByDock(DockPosition::CenterTop);
        assert(left.size() == 1);    // file-tree
        assert(right.size() == 3);   // ast-view, annotations, properties
        assert(bottom.size() == 3);  // diagnostics, output, workflow
        assert(center.size() == 1);  // code-editor
        assert(left[0].id == "file-tree");
        assert(center[0].id == "code-editor");
        std::cout << "Test 3 PASSED: Panels in correct dock positions\n";
        passed++;
    }

    // Test 4: Toggle panel visibility
    {
        PanelManager pm;
        pm.registerDefaults();
        assert(pm.isVisible("ast-view"));
        pm.togglePanel("ast-view");
        assert(!pm.isVisible("ast-view"));
        pm.togglePanel("ast-view");
        assert(pm.isVisible("ast-view"));
        std::cout << "Test 4 PASSED: Toggle panel visibility\n";
        passed++;
    }

    // Test 5: Hidden panel not in visible list
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.setVisible("properties", false);
        auto visible = pm.getVisiblePanels();
        assert(visible.size() == 7);
        bool found = false;
        for (const auto& p : visible) {
            if (p.id == "properties") found = true;
        }
        assert(!found);
        std::cout << "Test 5 PASSED: Hidden panel excluded from visible list\n";
        passed++;
    }

    // Test 6: Focus panel
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.focusPanel("diagnostics");
        assert(pm.getFocusedPanel() == "diagnostics");
        pm.focusPanel("ast-view");
        assert(pm.getFocusedPanel() == "ast-view");
        std::cout << "Test 6 PASSED: Focus panel tracking\n";
        passed++;
    }

    // Test 7: Move panel to different dock
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.movePanelToDock("diagnostics", DockPosition::Right);
        auto right = pm.getPanelsByDock(DockPosition::Right);
        bool found = false;
        for (const auto& p : right) {
            if (p.id == "diagnostics") found = true;
        }
        assert(found);
        auto bottom = pm.getPanelsByDock(DockPosition::Bottom);
        bool stillInBottom = false;
        for (const auto& p : bottom) {
            if (p.id == "diagnostics") stillInBottom = true;
        }
        assert(!stillInBottom);
        std::cout << "Test 7 PASSED: Move panel to different dock\n";
        passed++;
    }

    // Test 8: DockingLayout default proportions
    {
        PanelManager pm;
        pm.registerDefaults();
        assert(pm.layout().leftWidth == 20.0f);
        assert(pm.layout().rightWidth == 20.0f);
        assert(pm.layout().bottomHeight == 25.0f);
        std::cout << "Test 8 PASSED: Default layout proportions\n";
        passed++;
    }

    // Test 9: Panel ordering preserved
    {
        PanelManager pm;
        pm.registerDefaults();
        auto all = pm.getPanelList();
        assert(all[0].id == "file-tree");
        assert(all[1].id == "code-editor");
        // Higher order panels come later
        assert(all.back().id == "workflow");
        std::cout << "Test 9 PASSED: Panel ordering preserved\n";
        passed++;
    }

    // Test 10: JSON roundtrip — panels + layout
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.setVisible("workflow", false);
        pm.focusPanel("code-editor");
        pm.layout().leftWidth = 25.0f;

        nlohmann::json j = pm.toJson();
        PanelManager restored = PanelManager::fromJson(j);

        assert(restored.panelCount() == 8);
        assert(!restored.isVisible("workflow"));
        assert(restored.getFocusedPanel() == "code-editor");
        assert(restored.layout().leftWidth == 25.0f);
        std::cout << "Test 10 PASSED: JSON roundtrip (panels + layout)\n";
        passed++;
    }

    // Test 11: Panel info fields correct
    {
        PanelManager pm;
        pm.registerDefaults();
        auto info = pm.getPanel("file-tree");
        assert(info.id == "file-tree");
        assert(info.title == "File Tree");
        assert(info.defaultDock == DockPosition::Left);
        assert(info.category == "navigation");
        assert(!info.icon.empty());
        std::cout << "Test 11 PASSED: Panel info fields correct\n";
        passed++;
    }

    // Test 12: DockPosition string roundtrip
    {
        assert(dockPositionToString(DockPosition::Left) == "left");
        assert(dockPositionToString(DockPosition::CenterTop) == "center-top");
        assert(dockPositionToString(DockPosition::Bottom) == "bottom");
        assert(dockPositionFromString("left") == DockPosition::Left);
        assert(dockPositionFromString("right") == DockPosition::Right);
        assert(dockPositionFromString("center-top") == DockPosition::CenterTop);
        std::cout << "Test 12 PASSED: DockPosition string roundtrip\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
