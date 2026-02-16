// Step 350: Phase 13b Integration — Visual Consistency (8 tests)
// Tests theme+icons+syntax bridge all work together consistently

#include <cassert>
#include <iostream>
#include <string>
#include <set>
#include "ThemeData.h"
#include "SyntaxThemeBridge.h"
#include "Icons.h"
#include "PanelManager.h"

int main() {
    int passed = 0;

    // Test 1: All panels use theme background colors (no default grey)
    {
        auto theme = getDefaultDarkTheme();
        // Panel bg should not be default ImGui grey (0.27, 0.27, 0.28)
        assert(theme.bgPanel.toHex() != "#454548");
        assert(theme.bgPanel.toHex() != "#44444a");
        // bg, bgAlt, bgPanel all defined and distinct
        assert(theme.bg.toHex() != theme.bgPanel.toHex());
        assert(theme.bgAlt.toHex() != theme.bgPanel.toHex());
        std::cout << "Test 1 PASSED: Theme backgrounds defined (no default grey)\n";
        passed++;
    }

    // Test 2: Diagnostic panel severity icons + colors consistent
    {
        auto theme = getDefaultDarkTheme();
        // Error icon exists and error color is red-ish
        assert(std::string(WhetstoneIcons::DiagError).size() > 0);
        assert(theme.diagError.r > 0.5f);
        // Warning icon + amber color
        assert(std::string(WhetstoneIcons::DiagWarning).size() > 0);
        assert(theme.diagWarning.r > 0.5f && theme.diagWarning.g > 0.3f);
        // Info icon + blue color
        assert(std::string(WhetstoneIcons::DiagInfo).size() > 0);
        assert(theme.diagInfo.b > 0.5f);
        std::cout << "Test 2 PASSED: Diagnostic icons + theme colors consistent\n";
        passed++;
    }

    // Test 3: AST panel node type icons all defined
    {
        std::vector<std::string> nodeTypes = {
            "Function", "ClassDeclaration", "Variable", "Module",
            "EnumDeclaration", "NamespaceDeclaration"
        };
        for (const auto& nt : nodeTypes) {
            auto icon = WhetstoneIcons::iconForNodeType(nt);
            assert(std::string(icon).size() > 0);
        }
        std::cout << "Test 3 PASSED: AST node type icons all defined\n";
        passed++;
    }

    // Test 4: File tree uses folder/file icons
    {
        assert(std::string(WhetstoneIcons::Folder).size() > 0);
        assert(std::string(WhetstoneIcons::FolderOpen).size() > 0);
        assert(std::string(WhetstoneIcons::FileSource).size() > 0);
        assert(std::string(WhetstoneIcons::FileHeader).size() > 0);
        // Folder and file icons are different
        assert(std::string(WhetstoneIcons::Folder) != std::string(WhetstoneIcons::FileSource));
        std::cout << "Test 4 PASSED: File tree folder/file icons distinct\n";
        passed++;
    }

    // Test 5: Status text uses textDim for secondary info
    {
        auto theme = getDefaultDarkTheme();
        // textDim should be dimmer than text (lower luminance)
        float textLum = theme.text.r * 0.299f + theme.text.g * 0.587f + theme.text.b * 0.114f;
        float dimLum = theme.textDim.r * 0.299f + theme.textDim.g * 0.587f + theme.textDim.b * 0.114f;
        assert(dimLum < textLum);
        // Line numbers should use textDim via bridge
        auto lineNumColor = SyntaxThemeBridge::lineNumberColor(theme);
        assert(lineNumColor.toHex() == theme.textDim.toHex());
        std::cout << "Test 5 PASSED: textDim dimmer than text, used for secondary\n";
        passed++;
    }

    // Test 6: Focused panel border uses accent color
    {
        auto theme = getDefaultDarkTheme();
        assert(theme.borderFocused.toHex() == theme.accent.toHex());
        // Regular border different from focused
        assert(theme.border.toHex() != theme.borderFocused.toHex());
        std::cout << "Test 6 PASSED: Focused border = accent, distinct from regular\n";
        passed++;
    }

    // Test 7: Theme switch dark→light — all syntax bridge colors change
    {
        auto dark = getDefaultDarkTheme();
        auto light = getDefaultLightTheme();
        auto darkMappings = SyntaxThemeBridge::getAllMappings(dark);
        auto lightMappings = SyntaxThemeBridge::getAllMappings(light);
        int different = 0;
        for (const auto& [key, darkColor] : darkMappings) {
            auto it = lightMappings.find(key);
            if (it != lightMappings.end() && it->second.toHex() != darkColor.toHex()) {
                different++;
            }
        }
        assert(different >= 8); // most colors should change
        std::cout << "Test 7 PASSED: Theme switch changes " << different << "/12 syntax colors\n";
        passed++;
    }

    // Test 8: Full visual stack — panels + theme + icons + syntax bridge coherent
    {
        PanelManager pm;
        pm.registerDefaults();
        auto theme = getDefaultDarkTheme();
        auto mappings = SyntaxThemeBridge::getAllMappings(theme);

        // 8 panels
        assert(pm.panelCount() == 8);
        // 12 syntax color mappings
        assert(mappings.size() == 12);
        // All syntax colors readable against bg
        assert(SyntaxThemeBridge::allColorsReadable(theme));
        // Theme roundtrip stable
        auto j = theme.toJson();
        auto restored = WhetstoneTheme::fromJson(j);
        assert(restored.name == theme.name);
        assert(restored.accent.toHex() == theme.accent.toHex());
        // Icons available for all workflow states
        std::vector<std::string> states = {"pending", "in-progress", "complete", "rejected", "review"};
        for (const auto& s : states) {
            assert(std::string(WhetstoneIcons::iconForWorkflowStatus(s)).size() > 0);
        }

        std::cout << "Test 8 PASSED: Full visual stack coherent (panels+theme+icons+syntax)\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
