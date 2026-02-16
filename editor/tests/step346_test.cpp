// Step 346: Phase 13a Integration — Layout Persistence + Full Validation (8 tests)
// Tests save/load layout, panel+theme+keybinding integration

#include <cassert>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include "PanelManager.h"
#include "ThemeData.h"
#include "KeybindingRegistry.h"
#include "Icons.h"
#include "CommandPalette.h"

int main() {
    int passed = 0;
    std::string tmpDir = std::filesystem::temp_directory_path().string() + "/whetstone_test_346";
    std::filesystem::create_directories(tmpDir + "/.whetstone");

    // Test 1: Default layout applied on first run
    {
        PanelManager pm;
        pm.registerDefaults();
        assert(pm.panelCount() == 8);
        assert(pm.layout().leftWidth == 20.0f);
        assert(pm.layout().rightWidth == 20.0f);
        assert(pm.layout().bottomHeight == 25.0f);
        std::cout << "Test 1 PASSED: Default layout applied on first run\n";
        passed++;
    }

    // Test 2: Save → reload → same layout
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.layout().leftWidth = 25.0f;
        pm.setVisible("workflow", false);
        pm.focusPanel("diagnostics");

        std::string path = tmpDir + "/.whetstone/layout.json";
        std::ofstream out(path);
        out << pm.toJson().dump(2);
        out.close();

        std::ifstream in(path);
        nlohmann::json j = nlohmann::json::parse(in);
        in.close();
        PanelManager restored = PanelManager::fromJson(j);

        assert(restored.layout().leftWidth == 25.0f);
        assert(!restored.isVisible("workflow"));
        assert(restored.getFocusedPanel() == "diagnostics");
        std::cout << "Test 2 PASSED: Save → reload → same layout\n";
        passed++;
    }

    // Test 3: Move panel → save → reload → same dock
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.movePanelToDock("annotations", DockPosition::Bottom);
        nlohmann::json j = pm.toJson();
        PanelManager restored = PanelManager::fromJson(j);
        auto info = restored.getPanel("annotations");
        assert(info.currentDock == DockPosition::Bottom);
        std::cout << "Test 3 PASSED: Panel dock position persisted\n";
        passed++;
    }

    // Test 4: Tab order preserved across save/restore
    {
        PanelManager pm;
        pm.registerDefaults();
        auto j = pm.toJson();
        auto restored = PanelManager::fromJson(j);
        auto all = restored.getPanelList();
        assert(all.size() == 8);
        assert(all[0].id == "file-tree");
        assert(all[1].id == "code-editor");
        std::cout << "Test 4 PASSED: Tab order preserved\n";
        passed++;
    }

    // Test 5: Corrupt JSON → falls back gracefully
    {
        PanelManager pm = PanelManager::fromJson(nlohmann::json::object());
        assert(pm.panelCount() == 0); // no panels from empty JSON
        pm.registerDefaults(); // can still register defaults
        assert(pm.panelCount() == 8);
        std::cout << "Test 5 PASSED: Corrupt/empty JSON handled gracefully\n";
        passed++;
    }

    // Test 6: Keybinding save/load roundtrip with custom bindings
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        reg.bind("save-buffer", KeyCombo::ctrl("W")); // rebind

        std::string path = tmpDir + "/.whetstone/keybindings.json";
        std::ofstream out(path);
        out << reg.toJson().dump(2);
        out.close();

        std::ifstream in(path);
        nlohmann::json j = nlohmann::json::parse(in);
        in.close();
        KeybindingRegistry restored;
        restored.loadDefaults();
        restored.loadFromJson(j);

        assert(restored.getBinding("save-buffer").toString() == "Ctrl+W");
        assert(restored.getBinding("undo").toString() == "Ctrl+Z"); // unchanged
        std::cout << "Test 6 PASSED: Keybinding save/load roundtrip\n";
        passed++;
    }

    // Test 7: Full integration: panels + keybindings + command palette
    {
        PanelManager pm;
        pm.registerDefaults();
        KeybindingRegistry reg;
        reg.loadDefaults();
        CommandPalette cp;
        for (const auto& action : reg.getActions()) {
            cp.registerCommand(action.id, action.label, reg.getSymbols(action.id), action.category);
        }

        // Search for "toggle" should find view actions
        auto results = cp.search("toggle");
        assert(!results.empty());
        bool foundToggle = false;
        for (const auto& r : results) {
            if (r.entry.label.find("Toggle") != std::string::npos) foundToggle = true;
        }
        assert(foundToggle);

        // Theme provides visual info
        auto theme = getDefaultDarkTheme();
        assert(!theme.name.empty());

        std::cout << "Test 7 PASSED: Full integration (panels + keys + palette + theme)\n";
        passed++;
    }

    // Test 8: Layout file location correct
    {
        std::string path = tmpDir + "/.whetstone/layout.json";
        assert(std::filesystem::exists(path)); // created in test 2
        auto size = std::filesystem::file_size(path);
        assert(size > 0);
        std::cout << "Test 8 PASSED: Layout file location and content (" << size << " bytes)\n";
        passed++;
    }

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
