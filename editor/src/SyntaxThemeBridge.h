#pragma once
// Step 348: Syntax Theme Bridge — maps TokenCategory to theme colors
// Connects the existing SyntaxHighlighter's TokenCategory enum to WhetstoneTheme colors
// Headless: no ImGui dependency. Provides Color4 values that renderers can apply.

#include <string>
#include <vector>
#include <map>
#include "ThemeData.h"

// TokenCategory duplicated here for headless testing (original is in SyntaxHighlighter.h
// which depends on tree-sitter). Enum values must match.
enum class TokenCategoryBridge {
    Plain, Keyword, String, Comment, Number, Identifier,
    Operator, Type, Punctuation, Function, Parameter, Builtin
};

struct ThemedSpan {
    uint32_t start;
    uint32_t end;
    TokenCategoryBridge category;
    Color4 color;
};

class SyntaxThemeBridge {
public:
    // Map a token category to its theme color
    static Color4 colorForCategory(TokenCategoryBridge cat, const WhetstoneTheme& theme) {
        switch (cat) {
            case TokenCategoryBridge::Keyword:     return theme.syntaxKeyword;
            case TokenCategoryBridge::String:       return theme.syntaxString;
            case TokenCategoryBridge::Comment:      return theme.syntaxComment;
            case TokenCategoryBridge::Number:       return theme.syntaxNumber;
            case TokenCategoryBridge::Function:     return theme.syntaxFunction;
            case TokenCategoryBridge::Type:         return theme.syntaxType;
            case TokenCategoryBridge::Operator:     return theme.syntaxOperator;
            case TokenCategoryBridge::Parameter:    return theme.syntaxAnnotation;
            case TokenCategoryBridge::Builtin:      return theme.syntaxFunction;
            case TokenCategoryBridge::Identifier:   return theme.text;
            case TokenCategoryBridge::Punctuation:  return theme.textDim;
            case TokenCategoryBridge::Plain:
            default:                                return theme.text;
        }
    }

    // Get all category-color mappings for a theme (useful for settings UI)
    static std::map<std::string, Color4> getAllMappings(const WhetstoneTheme& theme) {
        std::map<std::string, Color4> m;
        m["Keyword"]     = theme.syntaxKeyword;
        m["String"]      = theme.syntaxString;
        m["Comment"]     = theme.syntaxComment;
        m["Number"]      = theme.syntaxNumber;
        m["Function"]    = theme.syntaxFunction;
        m["Type"]        = theme.syntaxType;
        m["Operator"]    = theme.syntaxOperator;
        m["Parameter"]   = theme.syntaxAnnotation;
        m["Builtin"]     = theme.syntaxFunction;
        m["Identifier"]  = theme.text;
        m["Punctuation"] = theme.textDim;
        m["Plain"]       = theme.text;
        return m;
    }

    // Apply theme colors to a set of spans (returns ThemedSpans with colors filled in)
    static std::vector<ThemedSpan> applyTheme(
        const std::vector<std::pair<uint32_t, uint32_t>>& ranges,
        const std::vector<TokenCategoryBridge>& categories,
        const WhetstoneTheme& theme)
    {
        std::vector<ThemedSpan> result;
        size_t n = std::min(ranges.size(), categories.size());
        result.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            result.push_back({ranges[i].first, ranges[i].second,
                              categories[i], colorForCategory(categories[i], theme)});
        }
        return result;
    }

    // Get the line number color (always textDim)
    static Color4 lineNumberColor(const WhetstoneTheme& theme) { return theme.textDim; }

    // Get the current-line highlight color (bgAlt)
    static Color4 currentLineColor(const WhetstoneTheme& theme) { return theme.bgAlt; }

    // Get selection colors
    static Color4 selectionBgColor(const WhetstoneTheme& theme) { return theme.selectionBg; }
    static Color4 selectionTextColor(const WhetstoneTheme& theme) { return theme.selectionText; }

    // Semanno annotation color (distinct from regular comments)
    static Color4 semannoAnnotationColor(const WhetstoneTheme& theme) { return theme.syntaxAnnotation; }

    // Check if all syntax colors are distinct from background (readability check)
    static bool allColorsReadable(const WhetstoneTheme& theme) {
        auto bg = theme.bg;
        auto check = [&](const Color4& c) {
            float dr = c.r - bg.r, dg = c.g - bg.g, db = c.b - bg.b;
            float dist = dr*dr + dg*dg + db*db;
            return dist > 0.01f; // minimum contrast threshold
        };
        return check(theme.syntaxKeyword) && check(theme.syntaxString) &&
               check(theme.syntaxNumber) && check(theme.syntaxComment) &&
               check(theme.syntaxFunction) && check(theme.syntaxType) &&
               check(theme.syntaxOperator) && check(theme.syntaxAnnotation) &&
               check(theme.text) && check(theme.textDim);
    }

    // Get category name as string
    static std::string categoryName(TokenCategoryBridge cat) {
        switch (cat) {
            case TokenCategoryBridge::Keyword:     return "Keyword";
            case TokenCategoryBridge::String:       return "String";
            case TokenCategoryBridge::Comment:      return "Comment";
            case TokenCategoryBridge::Number:       return "Number";
            case TokenCategoryBridge::Function:     return "Function";
            case TokenCategoryBridge::Type:         return "Type";
            case TokenCategoryBridge::Operator:     return "Operator";
            case TokenCategoryBridge::Parameter:    return "Parameter";
            case TokenCategoryBridge::Builtin:      return "Builtin";
            case TokenCategoryBridge::Identifier:   return "Identifier";
            case TokenCategoryBridge::Punctuation:  return "Punctuation";
            case TokenCategoryBridge::Plain:
            default:                                return "Plain";
        }
    }
};
