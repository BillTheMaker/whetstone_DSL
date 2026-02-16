// Step 355: Phase 13c Integration — Keyboard Shortcuts Help Panel (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "KeybindingRegistry.h"
#include "CommandPalette.h"
#include "MenuBar.h"
#include "KeyboardShortcutsPanel.h"

int main() {
    int passed = 0;

    // Test 1: all default bindings displayed in shortcuts panel
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        KeyboardShortcutsPanel panel;
        panel.loadFromRegistry(reg);
        assert(!panel.rows().empty());
        assert(panel.rows().size() >= static_cast<size_t>(reg.bindingCount()));
        std::cout << "Test 1 PASSED: default bindings displayed\n";
        passed++;
    }

    // Test 2: Ctrl+S triggers save action
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        KeyCombo::InputState in;
        in.ctrl = true;
        in.key = "s";
        auto action = reg.processInput(in);
        assert(action.has_value());
        assert(action.value() == "save-buffer");
        std::cout << "Test 2 PASSED: Ctrl+S triggers save\n";
        passed++;
    }

    // Test 3: Ctrl+P opens command palette
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        CommandPalette palette;
        assert(!palette.isOpen());
        KeyCombo::InputState in;
        in.ctrl = true;
        in.key = "P";
        auto action = reg.processInput(in);
        if (action.has_value() && action.value() == "command-palette") {
            palette.open();
        }
        assert(palette.isOpen());
        std::cout << "Test 3 PASSED: Ctrl+P opens palette\n";
        passed++;
    }

    // Test 4: command palette shows key symbols for registered actions
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        CommandPalette palette;
        palette.registerFromKeybindings(reg);
        auto all = palette.all();
        assert(!all.empty());
        int symbolCount = 0;
        for (const auto& c : all) {
            if (!c.shortcut.empty()) {
                assert(!c.symbols.empty());
                symbolCount++;
            }
        }
        assert(symbolCount >= 10);
        std::cout << "Test 4 PASSED: palette shows key symbols\n";
        passed++;
    }

    // Test 5: menu items show correct key symbols
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        auto menu = MenuBar::buildDefaults(reg);
        auto save = menu.findItem("save-buffer");
        auto undo = menu.findItem("undo");
        assert(!save.symbols.empty());
        assert(!undo.symbols.empty());
        std::cout << "Test 5 PASSED: menu symbols correct\n";
        passed++;
    }

    // Test 6: rebind action updates shortcuts panel
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        KeyboardShortcutsPanel panel;
        panel.loadFromRegistry(reg);
        std::string before;
        for (const auto& row : panel.rows()) {
            if (row.actionId == "save-buffer") before = row.symbols;
        }
        reg.bind("save-buffer", KeyCombo::ctrl("W"));
        panel.loadFromRegistry(reg);
        std::string after;
        for (const auto& row : panel.rows()) {
            if (row.actionId == "save-buffer") after = row.symbols;
        }
        assert(before != after);
        assert(after.find("W") != std::string::npos);
        std::cout << "Test 6 PASSED: rebind updates panel\n";
        passed++;
    }

    // Test 7: key symbols render on current platform mode
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        KeyboardShortcutsPanel panel;
        panel.loadFromRegistry(reg);
        bool hasAnySymbol = false;
        for (const auto& row : panel.rows()) {
            if (!row.symbols.empty()) {
                hasAnySymbol = true;
                break;
            }
        }
        assert(hasAnySymbol);
        std::cout << "Test 7 PASSED: platform symbols render\n";
        passed++;
    }

    // Test 8: full keyboard workflow Ctrl+P -> "pipeline" -> Enter -> run-pipeline
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        CommandPalette palette;
        palette.registerFromKeybindings(reg);

        bool pipelineRan = false;
        KeyCombo::InputState openIn;
        openIn.ctrl = true;
        openIn.key = "p";
        auto openAction = reg.processInput(openIn);
        assert(openAction.has_value());
        assert(openAction.value() == "command-palette");
        palette.open();
        assert(palette.isOpen());

        auto matches = palette.search("pipeline");
        assert(!matches.empty());
        auto selected = palette.executeSelected(matches);
        assert(selected.has_value());
        if (selected.value() == "run-pipeline") pipelineRan = true;
        assert(pipelineRan);
        std::cout << "Test 8 PASSED: keyboard workflow executes pipeline\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
