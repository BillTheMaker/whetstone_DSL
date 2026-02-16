// Step 345: Theme Data + Tabbed Panel Logic (12 tests)
// Tests WhetstoneTheme, Color4, dark/light themes, and tabbed panel state

#include <cassert>
#include <iostream>
#include <string>
#include "ThemeData.h"
#include "PanelManager.h"

int main() {
    int passed = 0;

    // Test 1: Dark theme applies — all semantic colors defined
    {
        auto theme = getDefaultDarkTheme();
        assert(theme.name == "Whetstone Dark");
        assert(!theme.bg.isPureBlack());
        assert(!theme.text.isPureWhite());
        assert(!theme.bgAlt.isPureBlack());
        std::cout << "Test 1 PASSED: Dark theme colors defined\n";
        passed++;
    }

    // Test 2: Light theme alternative
    {
        auto theme = getDefaultLightTheme();
        assert(theme.name == "Whetstone Light");
        assert(!theme.bg.isPureWhite()); // off-white, not pure
        assert(!theme.text.isPureBlack()); // near-black
        std::cout << "Test 2 PASSED: Light theme alternative\n";
        passed++;
    }

    // Test 3: Color4 hex roundtrip
    {
        Color4 c = Color4::fromHex("#7aa2f7");
        std::string hex = c.toHex();
        assert(hex == "#7aa2f7");

        Color4 c2 = Color4::fromHex("#ff9e64");
        assert(c2.toHex() == "#ff9e64");
        std::cout << "Test 3 PASSED: Color4 hex roundtrip\n";
        passed++;
    }

    // Test 4: Syntax colors distinct from each other
    {
        auto theme = getDefaultDarkTheme();
        // At least keyword, string, number, comment should be different
        assert(theme.syntaxKeyword.toHex() != theme.syntaxString.toHex());
        assert(theme.syntaxString.toHex() != theme.syntaxNumber.toHex());
        assert(theme.syntaxNumber.toHex() != theme.syntaxComment.toHex());
        assert(theme.syntaxComment.toHex() != theme.syntaxKeyword.toHex());
        std::cout << "Test 4 PASSED: Syntax colors distinct\n";
        passed++;
    }

    // Test 5: Diagnostic colors match severity (error = red-ish, warning = amber-ish)
    {
        auto theme = getDefaultDarkTheme();
        // Error should be red-ish (high r, low g)
        assert(theme.diagError.r > 0.5f);
        // Warning should be amber-ish (high r, medium g)
        assert(theme.diagWarning.r > 0.5f);
        assert(theme.diagWarning.g > 0.3f);
        // Info should be blue-ish
        assert(theme.diagInfo.b > 0.5f);
        std::cout << "Test 5 PASSED: Diagnostic colors appropriate for severity\n";
        passed++;
    }

    // Test 6: Font sizes reasonable
    {
        auto theme = getDefaultDarkTheme();
        assert(theme.fontSizePrimary >= 10.0f && theme.fontSizePrimary <= 24.0f);
        assert(theme.fontSizeUI >= 10.0f && theme.fontSizeUI <= 20.0f);
        assert(theme.fontSizeSmall >= 8.0f && theme.fontSizeSmall <= 14.0f);
        std::cout << "Test 6 PASSED: Font sizes reasonable\n";
        passed++;
    }

    // Test 7: Theme JSON serialization
    {
        auto theme = getDefaultDarkTheme();
        auto j = theme.toJson();
        assert(j["name"] == "Whetstone Dark");
        assert(j.contains("bg"));
        assert(j.contains("accent"));
        assert(j.contains("syntaxKeyword"));
        assert(j.contains("fontSizePrimary"));
        std::cout << "Test 7 PASSED: Theme JSON serialization\n";
        passed++;
    }

    // Test 8: Accent color used for focused elements
    {
        auto theme = getDefaultDarkTheme();
        assert(theme.accent.toHex() == theme.borderFocused.toHex());
        assert(theme.accent.toHex() == theme.textAccent.toHex());
        std::cout << "Test 8 PASSED: Accent color consistent\n";
        passed++;
    }

    // Test 9: Tabbed bottom panel — hide one tab, others remain
    {
        PanelManager pm;
        pm.registerDefaults();
        pm.setVisible("workflow", false);
        auto bottom = pm.getPanelsByDock(DockPosition::Bottom);
        assert(bottom.size() == 2); // diagnostics + output
        std::cout << "Test 9 PASSED: Hide tab removes from dock\n";
        passed++;
    }

    // Test 10: Tab ordering in bottom panel
    {
        PanelManager pm;
        pm.registerDefaults();
        auto bottom = pm.getPanelsByDock(DockPosition::Bottom);
        // Should be ordered by order field: diagnostics(5), output(6), workflow(7)
        assert(bottom[0].id == "diagnostics");
        assert(bottom[1].id == "output");
        assert(bottom[2].id == "workflow");
        std::cout << "Test 10 PASSED: Tab ordering in bottom dock\n";
        passed++;
    }

    // Test 11: Dark vs light contrast — text vs background
    {
        auto dark = getDefaultDarkTheme();
        // Dark theme: light text on dark bg
        assert(dark.text.r > dark.bg.r);  // text lighter than bg
        assert(dark.text.g > dark.bg.g);

        auto light = getDefaultLightTheme();
        // Light theme: dark text on light bg
        assert(light.text.r < light.bg.r);
        assert(light.text.g < light.bg.g);
        std::cout << "Test 11 PASSED: Dark/light contrast correct\n";
        passed++;
    }

    // Test 12: Color4 component access
    {
        Color4 c(0.5f, 0.6f, 0.7f, 0.8f);
        assert(c.r == 0.5f);
        assert(c.g == 0.6f);
        assert(c.b == 0.7f);
        assert(c.a == 0.8f);
        assert(!c.isPureBlack());
        assert(!c.isPureWhite());
        Color4 black(0, 0, 0);
        assert(black.isPureBlack());
        Color4 white(1, 1, 1);
        assert(white.isPureWhite());
        std::cout << "Test 12 PASSED: Color4 component access\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
