// Step 344: File Tree + Tabbed Panels + Command Palette Integration (12 tests)
// Tests FileTree data model integration with PanelManager, CommandPalette fuzzy search,
// and tabbed bottom panel logic

#include <cassert>
#include <iostream>
#include <string>
#include "PanelManager.h"
#include "Icons.h"
#include "KeybindingRegistry.h"
#include "CommandPalette.h"

int main() {
    int passed = 0;

    // Test 1: Bottom dock has 3 tabbed panels
    {
        PanelManager pm;
        pm.registerDefaults();
        auto bottom = pm.getPanelsByDock(DockPosition::Bottom);
        assert(bottom.size() == 3);
        // Verify correct panels: diagnostics, output, workflow
        std::vector<std::string> names;
        for (const auto& p : bottom) names.push_back(p.id);
        assert(std::find(names.begin(), names.end(), "diagnostics") != names.end());
        assert(std::find(names.begin(), names.end(), "output") != names.end());
        assert(std::find(names.begin(), names.end(), "workflow") != names.end());
        std::cout << "Test 1 PASSED: Bottom dock has 3 tabbed panels\n";
        passed++;
    }

    // Test 2: Right dock has 3 tabbed panels
    {
        PanelManager pm;
        pm.registerDefaults();
        auto right = pm.getPanelsByDock(DockPosition::Right);
        assert(right.size() == 3);
        std::vector<std::string> names;
        for (const auto& p : right) names.push_back(p.id);
        assert(std::find(names.begin(), names.end(), "ast-view") != names.end());
        assert(std::find(names.begin(), names.end(), "annotations") != names.end());
        assert(std::find(names.begin(), names.end(), "properties") != names.end());
        std::cout << "Test 2 PASSED: Right dock has 3 tabbed panels\n";
        passed++;
    }

    // Test 3: CommandPalette fuzzy search works
    {
        CommandPalette cp;
        cp.registerCommand("save-buffer", "Save Buffer", "Ctrl+S", "File");
        cp.registerCommand("save-all", "Save All Buffers", "Ctrl+Shift+S", "File");
        cp.registerCommand("run-pipeline", "Run Pipeline", "F5", "Tools");
        cp.registerCommand("toggle-ast", "Toggle AST View", "Ctrl+Shift+A", "View");

        auto results = cp.search("save");
        assert(results.size() >= 2); // both save commands
        assert(results[0].entry.label.find("Save") != std::string::npos);
        std::cout << "Test 3 PASSED: CommandPalette fuzzy search\n";
        passed++;
    }

    // Test 4: CommandPalette empty search returns all
    {
        CommandPalette cp;
        cp.registerCommand("a", "Alpha", "", "Cat1");
        cp.registerCommand("b", "Beta", "", "Cat2");
        cp.registerCommand("c", "Gamma", "", "Cat3");
        auto results = cp.search("");
        assert(results.size() == 3);
        std::cout << "Test 4 PASSED: Empty search returns all commands\n";
        passed++;
    }

    // Test 5: CommandPalette recent actions first
    {
        CommandPalette cp;
        cp.registerCommand("a", "Alpha", "", "Cat");
        cp.registerCommand("b", "Beta", "", "Cat");
        cp.registerCommand("c", "Gamma", "", "Cat");
        cp.markUsed("c");
        auto results = cp.search("");
        assert(results[0].entry.id == "c"); // most recent first
        std::cout << "Test 5 PASSED: Recent actions ranked first\n";
        passed++;
    }

    // Test 6: CommandPalette shortcut field populated
    {
        CommandPalette cp;
        cp.registerCommand("save", "Save Buffer", "Ctrl+S", "File");
        auto results = cp.search("save");
        assert(!results.empty());
        assert(results[0].entry.shortcut == "Ctrl+S");
        std::cout << "Test 6 PASSED: Shortcut field populated in results\n";
        passed++;
    }

    // Test 7: Keybinding → CommandPalette population
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        CommandPalette cp;
        // Populate from keybinding registry
        for (const auto& action : reg.getActions()) {
            std::string sym = reg.getSymbols(action.id);
            cp.registerCommand(action.id, action.label, sym, action.category);
        }
        auto all = cp.all();
        assert(all.size() >= 20);
        // Find save-buffer and verify shortcut
        bool foundSave = false;
        for (const auto& cmd : all) {
            if (cmd.id == "save-buffer") {
                foundSave = true;
                assert(cmd.shortcut == "Ctrl+S");
            }
        }
        assert(foundSave);
        std::cout << "Test 7 PASSED: Keybinding → CommandPalette population (" << all.size() << " commands)\n";
        passed++;
    }

    // Test 8: File type icons for different extensions
    {
        // Python → source, .h → header, .json → config
        assert(std::string(WhetstoneIcons::iconForNodeType("Function")) == WhetstoneIcons::NodeFunction);
        assert(std::string(WhetstoneIcons::iconForNodeType("ClassDeclaration")) == WhetstoneIcons::NodeClass);
        assert(std::string(WhetstoneIcons::iconForNodeType("EnumDeclaration")) == WhetstoneIcons::NodeClass);
        assert(std::string(WhetstoneIcons::iconForNodeType("NamespaceDeclaration")) == WhetstoneIcons::Folder);
        std::cout << "Test 8 PASSED: Node type icons correct\n";
        passed++;
    }

    // Test 9: Workflow status icons
    {
        assert(WhetstoneIcons::iconForWorkflowStatus("pending") == WhetstoneIcons::Pending);
        assert(WhetstoneIcons::iconForWorkflowStatus("in-progress") == WhetstoneIcons::InProgress);
        assert(WhetstoneIcons::iconForWorkflowStatus("complete") == WhetstoneIcons::Complete);
        assert(WhetstoneIcons::iconForWorkflowStatus("rejected") == WhetstoneIcons::Rejected);
        assert(WhetstoneIcons::iconForWorkflowStatus("review") == WhetstoneIcons::Review);
        std::cout << "Test 9 PASSED: Workflow status icons\n";
        passed++;
    }

    // Test 10: CommandPalette partial match
    {
        CommandPalette cp;
        cp.registerCommand("run-pipeline", "Run Pipeline", "F5", "Tools");
        cp.registerCommand("run-tests", "Run Tests", "", "Tools");
        cp.registerCommand("save-buffer", "Save Buffer", "Ctrl+S", "File");
        auto results = cp.search("run");
        assert(results.size() == 2);
        // Both run commands matched
        for (const auto& r : results) {
            assert(r.entry.label.find("Run") != std::string::npos);
        }
        std::cout << "Test 10 PASSED: Partial match works\n";
        passed++;
    }

    // Test 11: Panel hasPanel check
    {
        PanelManager pm;
        pm.registerDefaults();
        assert(pm.hasPanel("file-tree"));
        assert(pm.hasPanel("code-editor"));
        assert(pm.hasPanel("diagnostics"));
        assert(!pm.hasPanel("nonexistent"));
        std::cout << "Test 11 PASSED: hasPanel check\n";
        passed++;
    }

    // Test 12: Layout resize persists in JSON
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.layout().leftWidth = 30.0f;
        pm.layout().rightWidth = 15.0f;
        pm.layout().bottomHeight = 35.0f;

        auto j = pm.toJson();
        auto restored = PanelManager::fromJson(j);

        assert(restored.layout().leftWidth == 30.0f);
        assert(restored.layout().rightWidth == 15.0f);
        assert(restored.layout().bottomHeight == 35.0f);
        std::cout << "Test 12 PASSED: Layout resize persists in JSON\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
