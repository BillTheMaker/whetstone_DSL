// Step 354: Menu Bar with Key Symbols (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "MenuBar.h"
#include "KeybindingRegistry.h"

int main() {
    int passed = 0;
    KeybindingRegistry reg;
    reg.loadDefaults();
    MenuBar bar = MenuBar::buildDefaults(reg);

    // Test 1: all 5 core menus render (+ Help)
    {
        assert(bar.hasMenu("File"));
        assert(bar.hasMenu("Edit"));
        assert(bar.hasMenu("View"));
        assert(bar.hasMenu("Tools"));
        assert(bar.hasMenu("Workflow"));
        assert(bar.hasMenu("Help"));
        std::cout << "Test 1 PASSED: all menus render\n";
        passed++;
    }

    // Test 2: File menu Save has Ctrl+S
    {
        MenuItem save = bar.findItem("save-buffer");
        assert(save.id == "save-buffer");
        assert(save.shortcut == "Ctrl+S");
        std::cout << "Test 2 PASSED: File Save shortcut\n";
        passed++;
    }

    // Test 3: Edit menu Undo has Ctrl+Z
    {
        MenuItem undo = bar.findItem("undo");
        assert(undo.id == "undo");
        assert(undo.shortcut == "Ctrl+Z");
        std::cout << "Test 3 PASSED: Edit Undo shortcut\n";
        passed++;
    }

    // Test 4: View menu panel toggles present
    {
        assert(bar.hasItem("toggle-file-tree"));
        assert(bar.hasItem("toggle-ast-view"));
        assert(bar.hasItem("toggle-diagnostics"));
        assert(bar.hasItem("toggle-annotations"));
        std::cout << "Test 4 PASSED: View panel toggles present\n";
        passed++;
    }

    // Test 5: Tools menu run pipeline present
    {
        MenuItem run = bar.findItem("run-pipeline");
        assert(run.id == "run-pipeline");
        assert(run.label.find("Run") != std::string::npos);
        std::cout << "Test 5 PASSED: Tools run pipeline present\n";
        passed++;
    }

    // Test 6: menu item executes correct action id
    {
        bar.openMenu("File");
        auto selected = bar.selectItem("save-buffer");
        assert(selected.has_value());
        assert(selected.value() == "save-buffer");
        std::cout << "Test 6 PASSED: menu selection executes action\n";
        passed++;
    }

    // Test 7: key symbols visible on bound actions
    {
        MenuItem save = bar.findItem("save-buffer");
        MenuItem run = bar.findItem("run-pipeline");
        assert(!save.symbols.empty());
        assert(!run.symbols.empty());
        std::cout << "Test 7 PASSED: key symbols visible\n";
        passed++;
    }

    // Test 8: menu closes after selection
    {
        bar.openMenu("Edit");
        assert(bar.isMenuOpen("Edit"));
        auto selected = bar.selectItem("undo");
        assert(selected.has_value());
        assert(!bar.isAnyMenuOpen());
        std::cout << "Test 8 PASSED: menu closes after selection\n";
        passed++;
    }

    // Test 9: separators between groups
    {
        assert(bar.separatorCount("File") >= 2);
        assert(bar.separatorCount("Edit") >= 1);
        assert(bar.separatorCount("Tools") >= 1);
        std::cout << "Test 9 PASSED: separators between groups\n";
        passed++;
    }

    // Test 10: workflow menu items present
    {
        assert(bar.hasItem("create-workflow"));
        assert(bar.hasItem("route-all"));
        assert(bar.hasItem("view-ready-tasks"));
        assert(bar.hasItem("save-workflow"));
        std::cout << "Test 10 PASSED: workflow items present\n";
        passed++;
    }

    // Test 11: submenu rendering metadata
    {
        assert(bar.hasSubmenu("docs"));
        assert(bar.hasItem("docs-getting-started"));
        assert(bar.hasItem("docs-keybindings"));
        std::cout << "Test 11 PASSED: submenu metadata\n";
        passed++;
    }

    // Test 12: selecting separator/unknown is ignored
    {
        auto bad = bar.selectItem("not-found");
        assert(!bad.has_value());
        auto sep = bar.selectItem("");
        assert(!sep.has_value());
        std::cout << "Test 12 PASSED: invalid selections ignored\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
