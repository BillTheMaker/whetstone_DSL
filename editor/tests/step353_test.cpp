// Step 353: Command Palette (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "CommandPalette.h"
#include "KeybindingRegistry.h"
#include "PanelManager.h"

int main() {
    int passed = 0;

    // Test 1: open/close state
    {
        CommandPalette cp;
        assert(!cp.isOpen());
        cp.open();
        assert(cp.isOpen());
        cp.close();
        assert(!cp.isOpen());
        std::cout << "Test 1 PASSED: open/close state\n";
        passed++;
    }

    // Test 2: fuzzy search by action label
    {
        CommandPalette cp;
        cp.registerCommand("save-buffer", "Save Buffer", "Ctrl+S", "File");
        auto r = cp.search("svb");
        assert(!r.empty());
        assert(r[0].entry.id == "save-buffer");
        std::cout << "Test 2 PASSED: fuzzy search by label\n";
        passed++;
    }

    // Test 3: fuzzy search by alias
    {
        CommandPalette cp;
        cp.registerCommand("run-pipeline", "Run Pipeline", "F5", "Tools",
                           CommandContext_Any, "", false, {"build", "compile"});
        auto r = cp.search("bld");
        assert(!r.empty());
        assert(r[0].entry.id == "run-pipeline");
        std::cout << "Test 3 PASSED: fuzzy search by alias\n";
        passed++;
    }

    // Test 4: register from keybindings
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        CommandPalette cp;
        cp.registerFromKeybindings(reg);
        auto all = cp.all();
        assert(all.size() >= 20);
        bool hasSave = false;
        for (const auto& c : all) {
            if (c.id == "save-buffer") {
                hasSave = true;
                assert(c.shortcut == "Ctrl+S");
            }
        }
        assert(hasSave);
        std::cout << "Test 4 PASSED: register from keybindings\n";
        passed++;
    }

    // Test 5: register panel toggles
    {
        PanelManager pm;
        pm.registerDefaults();
        CommandPalette cp;
        cp.registerPanelToggles(pm);
        auto all = cp.all();
        bool hasFileTree = false;
        for (const auto& c : all) {
            if (c.id == "toggle-file-tree") {
                hasFileTree = true;
                assert(c.label.find("File Tree") != std::string::npos);
            }
        }
        assert(hasFileTree);
        std::cout << "Test 5 PASSED: register panel toggles\n";
        passed++;
    }

    // Test 6: enter executes selected action
    {
        CommandPalette cp;
        cp.registerCommand("open-file", "Open File", "Ctrl+O", "File");
        auto r = cp.search("");
        auto id = cp.executeSelected(r);
        assert(id.has_value());
        assert(id.value() == "open-file");
        std::cout << "Test 6 PASSED: execute selected action\n";
        passed++;
    }

    // Test 7: arrow navigation wraps
    {
        CommandPalette cp;
        cp.registerCommand("a", "Action A", "", "");
        cp.registerCommand("b", "Action B", "", "");
        auto r = cp.search("");
        assert(!r.empty());
        cp.moveSelection(-1, (int)r.size());
        assert(cp.selectedIndex() == (int)r.size() - 1);
        cp.moveSelection(1, (int)r.size());
        assert(cp.selectedIndex() == 0);
        std::cout << "Test 7 PASSED: arrow navigation wraps\n";
        passed++;
    }

    // Test 8: query reset resets selection
    {
        CommandPalette cp;
        cp.registerCommand("x", "Action X", "", "");
        cp.registerCommand("y", "Action Y", "", "");
        auto r = cp.search("");
        cp.moveSelection(1, (int)r.size());
        assert(cp.selectedIndex() == 1);
        cp.setQuery("a");
        assert(cp.selectedIndex() == 0);
        std::cout << "Test 8 PASSED: query reset resets selection\n";
        passed++;
    }

    // Test 9: recently used actions appear first on empty query
    {
        CommandPalette cp;
        cp.registerCommand("open", "Open", "", "");
        cp.registerCommand("save", "Save", "", "");
        cp.markUsed("save");
        auto r = cp.search("");
        assert(!r.empty());
        assert(r[0].entry.id == "save");
        std::cout << "Test 9 PASSED: recent actions first\n";
        passed++;
    }

    // Test 10: strict context filter
    {
        CommandPalette cp;
        cp.registerCommand("editor-only", "Editor Only", "", "",
                           CommandContext_Editor);
        cp.registerCommand("terminal-only", "Terminal Only", "", "",
                           CommandContext_Terminal);
        auto editor = cp.search("", CommandContext_Editor, true);
        assert(editor.size() == 1);
        assert(editor[0].entry.id == "editor-only");
        std::cout << "Test 10 PASSED: strict context filter\n";
        passed++;
    }

    // Test 11: search can match category text
    {
        CommandPalette cp;
        cp.registerCommand("run", "Run Pipeline", "F5", "Tools");
        auto r = cp.search("tls");
        assert(!r.empty());
        assert(r[0].entry.id == "run");
        std::cout << "Test 11 PASSED: category text search\n";
        passed++;
    }

    // Test 12: no-match query returns empty results
    {
        CommandPalette cp;
        cp.registerCommand("save", "Save Buffer", "Ctrl+S", "File");
        auto r = cp.search("zzzz");
        assert(r.empty());
        std::cout << "Test 12 PASSED: no-match query returns empty\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
