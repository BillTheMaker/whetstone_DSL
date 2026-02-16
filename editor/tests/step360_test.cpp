// Step 360: Phase 13d Integration + Sprint 13 Summary (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "PanelManager.h"
#include "ThemeData.h"
#include "KeybindingRegistry.h"
#include "KeySymbolRenderer.h"
#include "CommandPalette.h"
#include "MenuBar.h"
#include "KeyboardShortcutsPanel.h"
#include "panels/BreadcrumbBar.h"
#include "panels/StatusBar.h"
#include "panels/NavigationTools.h"
#include "panels/FindReplaceBar.h"
#include "ast/Function.h"

int main() {
    int passed = 0;
    WhetstoneTheme theme = getDefaultDarkTheme();

    // Shared module/project for navigation tests
    Module mod("m1", "app", "python");
    auto* helper = new Function("f1", "helper");
    helper->setSpan(9, 0, 12, 0);
    auto* caller = new Function("f2", "caller");
    caller->setSpan(19, 0, 24, 0);
    mod.addChild("functions", helper);
    mod.addChild("functions", caller);
    std::vector<std::pair<std::string, const Module*>> modules = {{"app.py", &mod}};

    // Test 1: keyboard workflow + definition + breadcrumb click-back
    {
        FunctionCall call;
        call.functionName = "helper";
        auto def = goToDefinition(&call, "app.py", modules);
        assert(def.has_value());
        auto crumbs = buildBreadcrumbBar(&mod, def->node, theme);
        assert(!crumbs.segments.empty());
        auto back = navigateBreadcrumbClick(crumbs, 0);
        assert(back == &mod);
        std::cout << "Test 1 PASSED: keyboard -> definition -> breadcrumb back\n";
        passed++;
    }

    // Test 2: command palette -> find -> project search -> navigate to result
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        CommandPalette palette;
        palette.registerFromKeybindings(reg);
        auto cmdMatches = palette.search("find");
        assert(!cmdMatches.empty());
        auto selected = palette.executeSelected(cmdMatches);
        assert(selected.has_value());
        assert(selected.value().find("find") != std::string::npos);

        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("helper", "helper()\n");
        auto results = bar.searchProject({{"app.py", "helper()\n"}, {"other.py", "x\n"}});
        assert(!results.empty());
        assert(results[0].filePath == "app.py");
        std::cout << "Test 2 PASSED: palette find -> project results\n";
        passed++;
    }

    // Test 3: status bar reflects buffer switches
    {
        StatusBarInput py;
        py.language = "python";
        py.filePath = "a.py";
        py.line = 4;
        py.column = 2;
        py.errorCount = 1;
        auto a = buildStatusBar(py, theme);

        StatusBarInput cpp;
        cpp.language = "cpp";
        cpp.filePath = "a.cpp";
        cpp.line = 30;
        cpp.column = 1;
        cpp.warningCount = 2;
        auto b = buildStatusBar(cpp, theme);

        assert(a.languageBadge != b.languageBadge);
        assert(a.centerText != b.centerText);
        assert(a.diagnosticsText != b.diagnosticsText);
        std::cout << "Test 3 PASSED: status bar reflects buffer switch\n";
        passed++;
    }

    // Test 4: all panels docked (no floating windows in model)
    {
        PanelManager pm;
        pm.registerDefaults();
        auto visible = pm.getVisiblePanels();
        assert(visible.size() == 8);
        for (const auto& p : visible) {
            assert(p.currentDock == DockPosition::Left ||
                   p.currentDock == DockPosition::CenterTop ||
                   p.currentDock == DockPosition::CenterBottom ||
                   p.currentDock == DockPosition::Right ||
                   p.currentDock == DockPosition::Bottom);
        }
        std::cout << "Test 4 PASSED: panels are docked\n";
        passed++;
    }

    // Test 5: theme consistency across panels/navigation elements
    {
        auto crumbs = buildBreadcrumbBar(&mod, helper, theme);
        StatusBarInput in;
        in.filePath = "app.py";
        auto status = buildStatusBar(in, theme);
        auto badge = KeySymbolRenderer::renderKeySymbol(KeyCombo::ctrl("S"), theme);
        assert(crumbs.textColor.toHex() == theme.text.toHex());
        assert(status.textColor.toHex() == theme.text.toHex());
        assert(badge.textColor.toHex() == theme.text.toHex());
        std::cout << "Test 5 PASSED: theme consistency\n";
        passed++;
    }

    // Test 6: key symbols visible in menu, palette, shortcuts panel
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        auto menu = MenuBar::buildDefaults(reg, theme);
        auto saveItem = menu.findItem("save-buffer");
        assert(!saveItem.symbols.empty());

        CommandPalette palette;
        palette.registerFromKeybindings(reg);
        bool paletteHasSymbol = false;
        for (const auto& entry : palette.all()) {
            if (entry.id == "save-buffer") {
                paletteHasSymbol = !entry.symbols.empty();
            }
        }
        assert(paletteHasSymbol);

        KeyboardShortcutsPanel panel;
        panel.loadFromRegistry(reg);
        bool panelHasSymbol = false;
        for (const auto& row : panel.rows()) {
            if (row.actionId == "save-buffer") panelHasSymbol = !row.symbols.empty();
        }
        assert(panelHasSymbol);
        std::cout << "Test 6 PASSED: key symbols visible everywhere\n";
        passed++;
    }

    // Test 7: layout save/restart/restore
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.setVisible("workflow", false);
        pm.movePanelToDock("diagnostics", DockPosition::Right);
        auto snap = pm.toJson();
        auto restored = PanelManager::fromJson(snap);
        assert(!restored.isVisible("workflow"));
        assert(restored.getPanel("diagnostics").currentDock == DockPosition::Right);
        std::cout << "Test 7 PASSED: layout restore\n";
        passed++;
    }

    // Test 8: Sprint 13 summary gates operational
    {
        bool dockingOk = true;
        bool themeOk = true;
        bool keysOk = true;
        bool navOk = true;

        PanelManager pm;
        pm.registerDefaults();
        dockingOk = pm.panelCount() >= 8;

        KeybindingRegistry reg;
        reg.loadDefaults();
        keysOk = reg.bindingCount() >= 15;

        auto st = buildStatusBar(StatusBarInput{"python", "app.py"}, theme);
        themeOk = (st.bgColor.toHex() == theme.bgPanel.toHex());

        auto symbols = collectFileSymbols(&mod, "app.py");
        navOk = !symbols.empty();

        assert(dockingOk && themeOk && keysOk && navOk);
        std::cout << "Test 8 PASSED: sprint summary gates\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
