// Step 348: Syntax Highlighting Theme Bridge (12 tests)
// Tests TokenCategory→Color4 mapping, themed spans, readability, semanno colors

#include <cassert>
#include <iostream>
#include <string>
#include <set>
#include "SyntaxThemeBridge.h"

int main() {
    int passed = 0;

    // Test 1: Keyword maps to syntaxKeyword color
    {
        auto theme = getDefaultDarkTheme();
        auto c = SyntaxThemeBridge::colorForCategory(TokenCategoryBridge::Keyword, theme);
        assert(c.toHex() == theme.syntaxKeyword.toHex());
        std::cout << "Test 1 PASSED: Keyword → syntaxKeyword\n";
        passed++;
    }

    // Test 2: All categories produce non-black colors
    {
        auto theme = getDefaultDarkTheme();
        std::vector<TokenCategoryBridge> cats = {
            TokenCategoryBridge::Plain, TokenCategoryBridge::Keyword,
            TokenCategoryBridge::String, TokenCategoryBridge::Comment,
            TokenCategoryBridge::Number, TokenCategoryBridge::Identifier,
            TokenCategoryBridge::Operator, TokenCategoryBridge::Type,
            TokenCategoryBridge::Punctuation, TokenCategoryBridge::Function,
            TokenCategoryBridge::Parameter, TokenCategoryBridge::Builtin
        };
        for (auto cat : cats) {
            auto c = SyntaxThemeBridge::colorForCategory(cat, theme);
            assert(!c.isPureBlack());
        }
        std::cout << "Test 2 PASSED: All 12 categories produce non-black colors\n";
        passed++;
    }

    // Test 3: String and Comment mapped to different colors
    {
        auto theme = getDefaultDarkTheme();
        auto str = SyntaxThemeBridge::colorForCategory(TokenCategoryBridge::String, theme);
        auto cmt = SyntaxThemeBridge::colorForCategory(TokenCategoryBridge::Comment, theme);
        assert(str.toHex() != cmt.toHex());
        std::cout << "Test 3 PASSED: String ≠ Comment color\n";
        passed++;
    }

    // Test 4: getAllMappings returns 12 entries
    {
        auto theme = getDefaultDarkTheme();
        auto map = SyntaxThemeBridge::getAllMappings(theme);
        assert(map.size() == 12);
        assert(map.count("Keyword") == 1);
        assert(map.count("String") == 1);
        assert(map.count("Plain") == 1);
        std::cout << "Test 4 PASSED: getAllMappings returns 12 entries\n";
        passed++;
    }

    // Test 5: applyTheme produces ThemedSpans with correct colors
    {
        auto theme = getDefaultDarkTheme();
        std::vector<std::pair<uint32_t, uint32_t>> ranges = {{0, 3}, {4, 10}, {11, 15}};
        std::vector<TokenCategoryBridge> cats = {
            TokenCategoryBridge::Keyword, TokenCategoryBridge::String, TokenCategoryBridge::Number
        };
        auto spans = SyntaxThemeBridge::applyTheme(ranges, cats, theme);
        assert(spans.size() == 3);
        assert(spans[0].color.toHex() == theme.syntaxKeyword.toHex());
        assert(spans[1].color.toHex() == theme.syntaxString.toHex());
        assert(spans[2].color.toHex() == theme.syntaxNumber.toHex());
        assert(spans[0].start == 0 && spans[0].end == 3);
        std::cout << "Test 5 PASSED: applyTheme produces correctly colored spans\n";
        passed++;
    }

    // Test 6: Line number color is textDim
    {
        auto theme = getDefaultDarkTheme();
        auto c = SyntaxThemeBridge::lineNumberColor(theme);
        assert(c.toHex() == theme.textDim.toHex());
        std::cout << "Test 6 PASSED: Line numbers use textDim\n";
        passed++;
    }

    // Test 7: Current line highlight is bgAlt
    {
        auto theme = getDefaultDarkTheme();
        auto c = SyntaxThemeBridge::currentLineColor(theme);
        assert(c.toHex() == theme.bgAlt.toHex());
        std::cout << "Test 7 PASSED: Current line uses bgAlt\n";
        passed++;
    }

    // Test 8: Selection colors correct
    {
        auto theme = getDefaultDarkTheme();
        assert(SyntaxThemeBridge::selectionBgColor(theme).toHex() == theme.selectionBg.toHex());
        assert(SyntaxThemeBridge::selectionTextColor(theme).toHex() == theme.selectionText.toHex());
        std::cout << "Test 8 PASSED: Selection colors correct\n";
        passed++;
    }

    // Test 9: Semanno annotation gets distinct color
    {
        auto theme = getDefaultDarkTheme();
        auto ann = SyntaxThemeBridge::semannoAnnotationColor(theme);
        auto cmt = SyntaxThemeBridge::colorForCategory(TokenCategoryBridge::Comment, theme);
        assert(ann.toHex() != cmt.toHex()); // annotation ≠ comment
        assert(ann.toHex() == theme.syntaxAnnotation.toHex());
        std::cout << "Test 9 PASSED: Semanno annotation color distinct from comment\n";
        passed++;
    }

    // Test 10: allColorsReadable — dark theme passes contrast check
    {
        auto theme = getDefaultDarkTheme();
        assert(SyntaxThemeBridge::allColorsReadable(theme));
        std::cout << "Test 10 PASSED: Dark theme passes readability check\n";
        passed++;
    }

    // Test 11: allColorsReadable — light theme passes too
    {
        auto theme = getDefaultLightTheme();
        assert(SyntaxThemeBridge::allColorsReadable(theme));
        std::cout << "Test 11 PASSED: Light theme passes readability check\n";
        passed++;
    }

    // Test 12: categoryName returns correct strings
    {
        assert(SyntaxThemeBridge::categoryName(TokenCategoryBridge::Keyword) == "Keyword");
        assert(SyntaxThemeBridge::categoryName(TokenCategoryBridge::String) == "String");
        assert(SyntaxThemeBridge::categoryName(TokenCategoryBridge::Comment) == "Comment");
        assert(SyntaxThemeBridge::categoryName(TokenCategoryBridge::Number) == "Number");
        assert(SyntaxThemeBridge::categoryName(TokenCategoryBridge::Function) == "Function");
        assert(SyntaxThemeBridge::categoryName(TokenCategoryBridge::Type) == "Type");
        assert(SyntaxThemeBridge::categoryName(TokenCategoryBridge::Plain) == "Plain");
        std::cout << "Test 12 PASSED: categoryName strings correct\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
