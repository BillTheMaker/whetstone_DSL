// Step 343: Panel Registration + Icons + Keybinding Foundation (12 tests)
// Tests PanelManager registration, Icons system, and KeybindingRegistry basics

#include <cassert>
#include <iostream>
#include <string>
#include "PanelManager.h"
#include "Icons.h"
#include "KeybindingRegistry.h"

int main() {
    int passed = 0;

    // Test 1: Panel categories correct
    {
        PanelManager pm;
        pm.registerDefaults();
        auto all = pm.getPanelList();
        int nav = 0, editor = 0, analysis = 0, output = 0;
        for (const auto& p : all) {
            if (p.category == "navigation") nav++;
            if (p.category == "editor") editor++;
            if (p.category == "analysis") analysis++;
            if (p.category == "output") output++;
        }
        assert(nav == 1);      // file-tree
        assert(editor == 1);   // code-editor
        assert(analysis == 3); // ast-view, annotations, properties
        assert(output == 3);   // diagnostics, output, workflow
        std::cout << "Test 1 PASSED: Panel categories correct\n";
        passed++;
    }

    // Test 2: Panel JSON roundtrip preserves all fields
    {
        PanelManager pm;
        pm.registerPanel("test-panel", "Test Panel", DockPosition::Right,
                         WhetstoneIcons::NodeFunction, "analysis", 10);
        pm.setVisible("test-panel", false);
        pm.movePanelToDock("test-panel", DockPosition::Bottom);

        auto j = pm.toJson();
        auto restored = PanelManager::fromJson(j);
        auto info = restored.getPanel("test-panel");
        assert(info.title == "Test Panel");
        assert(!info.isVisible);
        assert(info.currentDock == DockPosition::Bottom);
        assert(info.defaultDock == DockPosition::Right);
        assert(info.order == 10);
        std::cout << "Test 2 PASSED: Panel JSON roundtrip preserves all fields\n";
        passed++;
    }

    // Test 3: Icon constants defined and non-empty
    {
        assert(std::string(WhetstoneIcons::DiagError).size() > 0);
        assert(std::string(WhetstoneIcons::DiagWarning).size() > 0);
        assert(std::string(WhetstoneIcons::DiagInfo).size() > 0);
        assert(std::string(WhetstoneIcons::Pending).size() > 0);
        assert(std::string(WhetstoneIcons::Complete).size() > 0);
        assert(std::string(WhetstoneIcons::NodeFunction).size() > 0);
        assert(std::string(WhetstoneIcons::NodeClass).size() > 0);
        assert(std::string(WhetstoneIcons::Folder).size() > 0);
        std::cout << "Test 3 PASSED: Icon constants defined and non-empty\n";
        passed++;
    }

    // Test 4: Icon for node type
    {
        assert(WhetstoneIcons::iconForNodeType("Function") == WhetstoneIcons::NodeFunction);
        assert(WhetstoneIcons::iconForNodeType("ClassDeclaration") == WhetstoneIcons::NodeClass);
        assert(WhetstoneIcons::iconForNodeType("Variable") == WhetstoneIcons::NodeVariable);
        assert(WhetstoneIcons::iconForNodeType("Module") == WhetstoneIcons::NodeModule);
        assert(WhetstoneIcons::iconForNodeType("IntentAnnotation") == WhetstoneIcons::NodeAnnotation);
        std::cout << "Test 4 PASSED: Icon for node type\n";
        passed++;
    }

    // Test 5: Icon for severity
    {
        assert(WhetstoneIcons::iconForSeverity(1) == WhetstoneIcons::DiagError);
        assert(WhetstoneIcons::iconForSeverity(2) == WhetstoneIcons::DiagWarning);
        assert(WhetstoneIcons::iconForSeverity(3) == WhetstoneIcons::DiagInfo);
        assert(WhetstoneIcons::iconForSeverity(4) == WhetstoneIcons::DiagHint);
        std::cout << "Test 5 PASSED: Icon for severity\n";
        passed++;
    }

    // Test 6: Icon fallback text
    {
        assert(WhetstoneIcons::fallback(WhetstoneIcons::DiagError) == "[ERR]");
        assert(WhetstoneIcons::fallback(WhetstoneIcons::DiagWarning) == "[WARN]");
        assert(WhetstoneIcons::fallback(WhetstoneIcons::Complete) == "[OK]");
        assert(WhetstoneIcons::fallback(WhetstoneIcons::NodeFunction) == "fn");
        std::cout << "Test 6 PASSED: Icon fallback text\n";
        passed++;
    }

    // Test 7: KeybindingRegistry default bindings loaded
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        assert(reg.actionCount() >= 20);
        assert(reg.bindingCount() >= 14);
        std::cout << "Test 7 PASSED: Default bindings loaded (" << reg.actionCount()
                  << " actions, " << reg.bindingCount() << " bindings)\n";
        passed++;
    }

    // Test 8: KeyCombo toString
    {
        auto combo = KeyCombo::ctrl("S");
        assert(combo.toString() == "Ctrl+S");
        auto combo2 = KeyCombo::ctrlShift("Z");
        assert(combo2.toString() == "Ctrl+Shift+Z");
        auto combo3 = KeyCombo::fkey("F5");
        assert(combo3.toString() == "F5");
        std::cout << "Test 8 PASSED: KeyCombo toString\n";
        passed++;
    }

    // Test 9: Key symbols for save action
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        std::string sym = reg.getSymbols("save-buffer");
        assert(sym == "Ctrl+S");
        std::string undo = reg.getSymbols("undo");
        assert(undo == "Ctrl+Z");
        std::string pipeline = reg.getSymbols("run-pipeline");
        assert(pipeline == "F5");
        std::cout << "Test 9 PASSED: Key symbols for actions\n";
        passed++;
    }

    // Test 10: Conflict detection
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        // Ctrl+S is already bound to save-buffer
        auto conflict = reg.findConflict("close-buffer", KeyCombo::ctrl("S"));
        assert(conflict.has_value());
        assert(conflict.value() == "save-buffer");
        // Ctrl+Q is not bound
        auto noConflict = reg.findConflict("close-buffer", KeyCombo::ctrl("Q"));
        assert(!noConflict.has_value());
        std::cout << "Test 10 PASSED: Conflict detection\n";
        passed++;
    }

    // Test 11: Actions by category
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        auto fileActions = reg.getActionsByCategory("File");
        assert(fileActions.size() >= 3);
        auto editActions = reg.getActionsByCategory("Edit");
        assert(editActions.size() >= 4);
        auto viewActions = reg.getActionsByCategory("View");
        assert(viewActions.size() >= 4);
        std::cout << "Test 11 PASSED: Actions by category\n";
        passed++;
    }

    // Test 12: Keybinding JSON persistence roundtrip
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        // Rebind save
        reg.bind("save-buffer", KeyCombo::ctrl("W"));

        auto j = reg.toJson();
        KeybindingRegistry restored;
        restored.loadDefaults();
        restored.loadFromJson(j);

        auto combo = restored.getBinding("save-buffer");
        assert(combo.toString() == "Ctrl+W");
        // Other bindings still intact
        assert(restored.getBinding("undo").toString() == "Ctrl+Z");
        std::cout << "Test 12 PASSED: Keybinding JSON persistence roundtrip\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
