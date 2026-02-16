// Step 352: Key Symbol Rendering (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "KeySymbolRenderer.h"

int main() {
    int passed = 0;
    WhetstoneTheme theme = getDefaultDarkTheme();

    // Test 1: single key renders
    {
        KeyCombo combo = KeyCombo::fkey("F5");
        KeyBadge badge = KeySymbolRenderer::renderKeySymbol(combo, theme);
        assert(badge.text == "F5");
        std::cout << "Test 1 PASSED: single key renders\n";
        passed++;
    }

    // Test 2: modifier+key renders
    {
        KeyCombo combo = KeyCombo::ctrl("S");
        KeyBadge badge = KeySymbolRenderer::renderKeySymbol(combo, theme);
        assert(badge.text == "Ctrl+S");
        std::cout << "Test 2 PASSED: modifier+key renders\n";
        passed++;
    }

    // Test 3: platform symbols correct
    {
        KeyCombo combo = KeyCombo::ctrlShift("P");
        KeyBadge mac = KeySymbolRenderer::renderKeyBadgeSymbols(combo, theme, true);
        assert(mac.text.find("\xE2\x8C\x83") != std::string::npos); // ⌃
        assert(mac.text.find("\xE2\x87\xA7") != std::string::npos); // ⇧
        std::cout << "Test 3 PASSED: platform symbols\n";
        passed++;
    }

    // Test 4: menu item layout label left + key right data present
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        auto item = KeySymbolRenderer::renderActionWithKey("Save Buffer", "save-buffer", reg, theme, "File");
        assert(item.label == "Save Buffer");
        assert(item.shortcut == "Ctrl+S");
        assert(item.hasBadge);
        std::cout << "Test 4 PASSED: menu item layout data\n";
        passed++;
    }

    // Test 5: chord rendering
    {
        std::vector<KeyCombo> chord = {KeyCombo::ctrl("K"), KeyCombo::ctrl("S")};
        std::string text = KeySymbolRenderer::renderKeyChord(chord);
        assert(text == "Ctrl+K Ctrl+S");
        std::cout << "Test 5 PASSED: chord rendering\n";
        passed++;
    }

    // Test 6: key badge has background
    {
        KeyCombo combo = KeyCombo::ctrl("F");
        KeyBadge badge = KeySymbolRenderer::renderKeySymbol(combo, theme);
        assert(badge.bgColor.toHex() == theme.bgAlt.toHex());
        assert(badge.borderColor.toHex() == theme.border.toHex());
        std::cout << "Test 6 PASSED: key badge background/border\n";
        passed++;
    }

    // Test 7: theme colors used
    {
        KeyCombo combo = KeyCombo::ctrl("P");
        KeyBadge badge = KeySymbolRenderer::renderKeySymbol(combo, theme);
        assert(badge.textColor.toHex() == theme.text.toHex());
        assert(badge.fontSize == theme.fontSizeSmall);
        std::cout << "Test 7 PASSED: theme colors used\n";
        passed++;
    }

    // Test 8: empty binding shows nothing
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        auto item = KeySymbolRenderer::renderActionWithKey("Unknown", "no-such-action", reg, theme);
        assert(!item.hasBadge);
        assert(item.shortcut.empty());
        assert(item.symbols.empty());
        std::cout << "Test 8 PASSED: empty binding hidden\n";
        passed++;
    }

    // Test 9: special keys render symbols on mac
    {
        KeyCombo enter = KeyCombo::fkey("Enter");
        KeyCombo esc = KeyCombo::fkey("Escape");
        KeyCombo tab = KeyCombo::fkey("Tab");
        auto b1 = KeySymbolRenderer::renderKeyBadgeSymbols(enter, theme, true);
        auto b2 = KeySymbolRenderer::renderKeyBadgeSymbols(esc, theme, true);
        auto b3 = KeySymbolRenderer::renderKeyBadgeSymbols(tab, theme, true);
        assert(b1.text.find("\xE2\x86\xB5") != std::string::npos); // ↵
        assert(b2.text.find("\xE2\x8E\x8B") != std::string::npos); // ⎋
        assert(b3.text.find("\xE2\x87\xA5") != std::string::npos); // ⇥
        std::cout << "Test 9 PASSED: special keys render symbols\n";
        passed++;
    }

    // Test 10: long label does not trigger overlap risk with normal widths
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        WhetstoneTheme roomy = theme;
        roomy.fontSizeUI = 18.0f;
        auto item = KeySymbolRenderer::renderActionWithKey(
            "Very Long Menu Action Label For Save Buffer", "save-buffer", reg, roomy, "File");
        assert(!item.overlapRisk);
        std::cout << "Test 10 PASSED: long label no overlap risk\n";
        passed++;
    }

    // Test 11: chord symbols render
    {
        std::vector<KeyCombo> chord = {KeyCombo::ctrl("K"), KeyCombo::ctrlShift("S")};
        std::string syms = KeySymbolRenderer::renderKeyChordSymbols(chord, true);
        assert(syms.find("\xE2\x8C\x83") != std::string::npos); // ⌃
        assert(syms.find("K") != std::string::npos);
        assert(syms.find("S") != std::string::npos);
        std::cout << "Test 11 PASSED: chord symbols render\n";
        passed++;
    }

    // Test 12: shouldRender logic
    {
        KeyCombo none;
        KeyCombo set = KeyCombo::ctrl("B");
        assert(!KeySymbolRenderer::shouldRender(none));
        assert(KeySymbolRenderer::shouldRender(set));
        std::cout << "Test 12 PASSED: shouldRender\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
