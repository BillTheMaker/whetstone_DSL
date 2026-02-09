// Step 76 TDD Test: Layout presets and docking configuration
//
// Tests:
// 1. Three presets available (VSCode, Emacs, JetBrains)
// 2. Default preset is VSCode
// 3. VSCode layout: Explorer left 20%, Editor center, Panel bottom 20%
// 4. Emacs layout: full-frame editor, Explorer hidden, Minibuffer visible
// 5. JetBrains layout: Explorer left, Editor center, ToolWindow right, Panel bottom
// 6. Switching preset marks layout dirty
// 7. Preset names round-trip (string ↔ enum)
// 8. Save/load persistence round-trip
// 9. Panel visibility queries work correctly
// 10. getVisiblePanelIds returns only visible panels

#include <iostream>
#include <string>
#include <cassert>
#include <cstdio>
#include <algorithm>
#include "LayoutManager.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Three presets available ---
    {
        auto presets = LayoutManager::availablePresets();
        assert(presets.size() == 3 && "Should have 3 presets");
        assert(presets[0] == LayoutPreset::VSCode);
        assert(presets[1] == LayoutPreset::Emacs);
        assert(presets[2] == LayoutPreset::JetBrains);

        std::cout << "Test 1 PASS: Three presets available" << std::endl;
        ++passed;
    }

    // --- Test 2: Default preset is VSCode ---
    {
        LayoutManager mgr;
        assert(mgr.getPreset() == LayoutPreset::VSCode && "Default should be VSCode");
        assert(std::string(LayoutManager::presetName(mgr.getPreset())) == "VSCode");

        std::cout << "Test 2 PASS: Default preset is VSCode" << std::endl;
        ++passed;
    }

    // --- Test 3: VSCode layout ---
    {
        LayoutManager mgr;
        const auto& cfg = mgr.getConfig();
        assert(cfg.name == "VSCode");
        assert(cfg.panels.size() == 3);

        // Explorer: left, 20%
        assert(cfg.panels[0].id == "Explorer");
        assert(cfg.panels[0].direction == DockDirection::Left);
        assert(cfg.panels[0].sizeRatio >= 0.19f && cfg.panels[0].sizeRatio <= 0.21f);
        assert(cfg.panels[0].visible == true);

        // Editor: center
        assert(cfg.panels[1].id == "Editor");
        assert(cfg.panels[1].direction == DockDirection::Center);
        assert(cfg.panels[1].visible == true);

        // Panel: bottom, 20%
        assert(cfg.panels[2].id == "Panel");
        assert(cfg.panels[2].direction == DockDirection::Bottom);
        assert(cfg.panels[2].sizeRatio >= 0.19f && cfg.panels[2].sizeRatio <= 0.21f);
        assert(cfg.panels[2].visible == true);

        assert(cfg.showStatusBar == true);

        std::cout << "Test 3 PASS: VSCode layout correct" << std::endl;
        ++passed;
    }

    // --- Test 4: Emacs layout ---
    {
        LayoutManager mgr;
        mgr.setPreset(LayoutPreset::Emacs);
        const auto& cfg = mgr.getConfig();
        assert(cfg.name == "Emacs");

        // Explorer should be hidden
        const auto* explorer = mgr.findPanel("Explorer");
        assert(explorer != nullptr);
        assert(explorer->visible == false && "Emacs: Explorer should be hidden");

        // Editor should be visible and large
        const auto* editor = mgr.findPanel("Editor");
        assert(editor != nullptr);
        assert(editor->visible == true);
        assert(editor->sizeRatio >= 0.80f);

        // Minibuffer should be visible (Emacs-specific)
        const auto* minibuf = mgr.findPanel("Minibuffer");
        assert(minibuf != nullptr);
        assert(minibuf->visible == true);
        assert(minibuf->direction == DockDirection::Bottom);

        // Panel should be hidden by default
        assert(mgr.isPanelVisible("Panel") == false);

        std::cout << "Test 4 PASS: Emacs layout correct" << std::endl;
        ++passed;
    }

    // --- Test 5: JetBrains layout ---
    {
        LayoutManager mgr;
        mgr.setPreset(LayoutPreset::JetBrains);
        const auto& cfg = mgr.getConfig();
        assert(cfg.name == "JetBrains");
        assert(cfg.panels.size() == 4);

        // Has ToolWindow on the right
        const auto* tw = mgr.findPanel("ToolWindow");
        assert(tw != nullptr);
        assert(tw->direction == DockDirection::Right);
        assert(tw->visible == true);

        // Explorer left, Editor center, Panel bottom
        assert(mgr.isPanelVisible("Explorer") == true);
        assert(mgr.isPanelVisible("Editor") == true);
        assert(mgr.isPanelVisible("Panel") == true);

        std::cout << "Test 5 PASS: JetBrains layout correct" << std::endl;
        ++passed;
    }

    // --- Test 6: Switching preset marks layout dirty ---
    {
        LayoutManager mgr;
        mgr.clearDirty();
        assert(mgr.isLayoutDirty() == false);

        mgr.setPreset(LayoutPreset::Emacs);
        assert(mgr.isLayoutDirty() == true && "Should be dirty after switch");

        mgr.clearDirty();
        assert(mgr.isLayoutDirty() == false);

        // Switching to same preset should NOT mark dirty
        mgr.setPreset(LayoutPreset::Emacs);
        assert(mgr.isLayoutDirty() == false && "Same preset should not mark dirty");

        std::cout << "Test 6 PASS: Dirty flag works correctly" << std::endl;
        ++passed;
    }

    // --- Test 7: Preset names round-trip ---
    {
        for (auto p : LayoutManager::availablePresets()) {
            std::string name = LayoutManager::presetName(p);
            LayoutPreset roundTripped = LayoutManager::presetFromName(name);
            assert(roundTripped == p && "Name should round-trip");
        }

        // Unknown name defaults to VSCode
        assert(LayoutManager::presetFromName("Unknown") == LayoutPreset::VSCode);
        assert(LayoutManager::presetFromName("") == LayoutPreset::VSCode);

        std::cout << "Test 7 PASS: Preset name round-trip" << std::endl;
        ++passed;
    }

    // --- Test 8: Save/load persistence ---
    {
        const std::string tmpFile = "test_layout_76.tmp";

        LayoutManager mgr1;
        mgr1.setPreset(LayoutPreset::JetBrains);
        assert(mgr1.saveToFile(tmpFile) == true);

        LayoutManager mgr2;
        assert(mgr2.getPreset() == LayoutPreset::VSCode && "Should start as VSCode");
        assert(mgr2.loadFromFile(tmpFile) == true);
        assert(mgr2.getPreset() == LayoutPreset::JetBrains && "Should load JetBrains");

        // Cleanup
        std::remove(tmpFile.c_str());

        // Load from nonexistent file should fail gracefully
        assert(mgr2.loadFromFile("nonexistent_file.tmp") == false);

        std::cout << "Test 8 PASS: Save/load persistence" << std::endl;
        ++passed;
    }

    // --- Test 9: Panel ratio queries ---
    {
        LayoutManager mgr;
        assert(mgr.getPanelRatio("Explorer") >= 0.19f);
        assert(mgr.getPanelRatio("NonExistent") == 0.0f);

        std::cout << "Test 9 PASS: Panel ratio queries" << std::endl;
        ++passed;
    }

    // --- Test 10: getVisiblePanelIds ---
    {
        LayoutManager mgr;

        // VSCode: all 3 visible
        auto vscodeVisible = mgr.getVisiblePanelIds();
        assert(vscodeVisible.size() == 3);

        // Emacs: Editor + Minibuffer visible (Explorer and Panel hidden)
        mgr.setPreset(LayoutPreset::Emacs);
        auto emacsVisible = mgr.getVisiblePanelIds();
        assert(emacsVisible.size() == 2);
        assert(std::find(emacsVisible.begin(), emacsVisible.end(), "Editor") != emacsVisible.end());
        assert(std::find(emacsVisible.begin(), emacsVisible.end(), "Minibuffer") != emacsVisible.end());

        // JetBrains: all 4 visible
        mgr.setPreset(LayoutPreset::JetBrains);
        auto jbVisible = mgr.getVisiblePanelIds();
        assert(jbVisible.size() == 4);

        std::cout << "Test 10 PASS: getVisiblePanelIds" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 76 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
