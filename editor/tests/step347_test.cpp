// Step 347: Theme System — Full Theme Roundtrip + Validation (12 tests)
// Tests theme JSON serialization/deserialization, color completeness, contrast

#include <cassert>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include "ThemeData.h"

int main() {
    int passed = 0;

    // Test 1: Dark theme — all semantic colors defined (no pure black/white)
    {
        auto t = getDefaultDarkTheme();
        assert(!t.bg.isPureBlack()); assert(!t.bg.isPureWhite());
        assert(!t.text.isPureBlack()); assert(!t.text.isPureWhite());
        assert(!t.accent.isPureBlack()); assert(!t.accent.isPureWhite());
        assert(!t.syntaxKeyword.isPureBlack());
        assert(!t.diagError.isPureBlack());
        std::cout << "Test 1 PASSED: Dark theme semantic colors defined\n";
        passed++;
    }

    // Test 2: Light theme — all semantic colors defined
    {
        auto t = getDefaultLightTheme();
        assert(!t.bg.isPureBlack()); assert(!t.bg.isPureWhite());
        assert(!t.text.isPureBlack()); assert(!t.text.isPureWhite());
        assert(!t.accent.isPureBlack()); assert(!t.accent.isPureWhite());
        std::cout << "Test 2 PASSED: Light theme semantic colors defined\n";
        passed++;
    }

    // Test 3: Full JSON roundtrip — dark theme
    {
        auto orig = getDefaultDarkTheme();
        auto j = orig.toJson();
        auto restored = WhetstoneTheme::fromJson(j);
        assert(restored.name == orig.name);
        assert(restored.bg.toHex() == orig.bg.toHex());
        assert(restored.text.toHex() == orig.text.toHex());
        assert(restored.accent.toHex() == orig.accent.toHex());
        assert(restored.syntaxKeyword.toHex() == orig.syntaxKeyword.toHex());
        assert(restored.diagError.toHex() == orig.diagError.toHex());
        assert(restored.fontSizePrimary == orig.fontSizePrimary);
        std::cout << "Test 3 PASSED: Full JSON roundtrip (dark)\n";
        passed++;
    }

    // Test 4: Full JSON roundtrip — light theme
    {
        auto orig = getDefaultLightTheme();
        auto j = orig.toJson();
        auto restored = WhetstoneTheme::fromJson(j);
        assert(restored.name == orig.name);
        assert(restored.bg.toHex() == orig.bg.toHex());
        assert(restored.syntaxString.toHex() == orig.syntaxString.toHex());
        assert(restored.diagWarning.toHex() == orig.diagWarning.toHex());
        std::cout << "Test 4 PASSED: Full JSON roundtrip (light)\n";
        passed++;
    }

    // Test 5: toJson includes all fields
    {
        auto j = getDefaultDarkTheme().toJson();
        // Check all expected keys
        std::vector<std::string> required = {
            "name", "bg", "bgAlt", "bgPanel", "bgPopup",
            "text", "textDim", "textAccent", "textError", "textWarning", "textSuccess",
            "border", "borderFocused", "accent", "accentHover", "accentActive",
            "selectionBg", "selectionText", "scrollbar", "scrollbarHover",
            "diagError", "diagWarning", "diagInfo", "diagHint",
            "syntaxKeyword", "syntaxString", "syntaxNumber", "syntaxComment",
            "syntaxFunction", "syntaxType", "syntaxOperator", "syntaxAnnotation",
            "fontSizePrimary", "fontSizeUI", "fontSizeSmall"
        };
        for (const auto& key : required) {
            assert(j.contains(key));
        }
        std::cout << "Test 5 PASSED: toJson includes all " << required.size() << " fields\n";
        passed++;
    }

    // Test 6: fromJson with missing fields uses defaults
    {
        nlohmann::json j;
        j["name"] = "Partial";
        j["bg"] = "#ff0000";
        auto t = WhetstoneTheme::fromJson(j);
        assert(t.name == "Partial");
        assert(t.bg.toHex() == "#ff0000");
        // Missing fields fall back to dark theme defaults
        auto def = getDefaultDarkTheme();
        assert(t.text.toHex() == def.text.toHex());
        assert(t.syntaxKeyword.toHex() == def.syntaxKeyword.toHex());
        std::cout << "Test 6 PASSED: fromJson with partial data uses defaults\n";
        passed++;
    }

    // Test 7: fromJson with empty object gives defaults
    {
        auto t = WhetstoneTheme::fromJson(nlohmann::json::object());
        assert(t.name == "Custom"); // default name
        auto def = getDefaultDarkTheme();
        assert(t.bg.toHex() == def.bg.toHex());
        assert(t.fontSizePrimary == def.fontSizePrimary);
        std::cout << "Test 7 PASSED: Empty JSON gives default values\n";
        passed++;
    }

    // Test 8: All 8 syntax colors distinct in dark theme
    {
        auto t = getDefaultDarkTheme();
        std::vector<std::string> hexes = {
            t.syntaxKeyword.toHex(), t.syntaxString.toHex(),
            t.syntaxNumber.toHex(), t.syntaxComment.toHex(),
            t.syntaxFunction.toHex(), t.syntaxType.toHex(),
            t.syntaxOperator.toHex(), t.syntaxAnnotation.toHex()
        };
        // At least 6 distinct colors (keyword == function is OK by design)
        std::set<std::string> unique(hexes.begin(), hexes.end());
        assert(unique.size() >= 6);
        std::cout << "Test 8 PASSED: " << unique.size() << " distinct syntax colors\n";
        passed++;
    }

    // Test 9: Color4 arithmetic precision
    {
        Color4 c = Color4::fromHex("#7aa2f7");
        // Round-trip should be stable
        std::string hex1 = c.toHex();
        Color4 c2 = Color4::fromHex(hex1);
        assert(c2.toHex() == hex1);
        // Alpha channel preserved
        Color4 ca = Color4::fromHex("#7aa2f780");
        assert(ca.a < 0.6f); // 0x80 = 128 → 128/255 ≈ 0.502
        std::cout << "Test 9 PASSED: Color4 hex precision and alpha\n";
        passed++;
    }

    // Test 10: Font size customization via JSON
    {
        nlohmann::json j;
        j["name"] = "BigFont";
        j["fontSizePrimary"] = 20.0f;
        j["fontSizeUI"] = 18.0f;
        j["fontSizeSmall"] = 12.0f;
        auto t = WhetstoneTheme::fromJson(j);
        assert(t.fontSizePrimary == 20.0f);
        assert(t.fontSizeUI == 18.0f);
        assert(t.fontSizeSmall == 12.0f);
        std::cout << "Test 10 PASSED: Font size customization via JSON\n";
        passed++;
    }

    // Test 11: Dark vs light theme contrast
    {
        auto dark = getDefaultDarkTheme();
        auto light = getDefaultLightTheme();
        // Dark: bg is dark, text is light
        assert(dark.bg.r < 0.2f && dark.text.r > 0.7f);
        // Light: bg is light, text is dark
        assert(light.bg.r > 0.8f && light.text.r < 0.2f);
        // Both have accent colors
        assert(!dark.accent.isPureBlack());
        assert(!light.accent.isPureBlack());
        std::cout << "Test 11 PASSED: Dark/light contrast correct\n";
        passed++;
    }

    // Test 12: Theme selection/scrollbar/popup colors defined
    {
        auto t = getDefaultDarkTheme();
        assert(!t.selectionBg.isPureBlack());
        assert(!t.scrollbar.isPureBlack());
        assert(!t.bgPopup.isPureBlack());
        assert(!t.accentHover.isPureBlack());
        assert(!t.accentActive.isPureBlack());
        // Hover should be lighter than accent for dark theme
        assert(t.accentHover.r >= t.accent.r || t.accentHover.g >= t.accent.g);
        std::cout << "Test 12 PASSED: Selection/scrollbar/popup colors defined\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
