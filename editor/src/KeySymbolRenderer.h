#pragma once
// Step 352: Key Symbol Renderer — headless key badge data model
// Produces styled key symbol text for menus, tooltips, and command palette
// No ImGui dependency — outputs text + style info that renderers can use

#include <string>
#include <vector>
#include "KeybindingRegistry.h"
#include "ThemeData.h"

struct KeyBadge {
    std::string text;       // e.g. "Ctrl+S" or "⌃S"
    Color4 textColor;
    Color4 bgColor;
    Color4 borderColor;
    float fontSize;
    float paddingX = 6.0f;
    float paddingY = 2.0f;
    float cornerRadius = 4.0f;
};

struct MenuItemLayout {
    std::string label;      // e.g. "Save Buffer"
    std::string shortcut;   // e.g. "Ctrl+S"
    std::string symbols;    // e.g. "⌃S"
    std::string category;
    bool hasBadge;
    bool overlapRisk = false;
    int minGapSpaces = 4;
};

class KeySymbolRenderer {
public:
    // Render a key badge for a key combo (text form).
    static KeyBadge renderKeySymbol(const KeyCombo& combo, const WhetstoneTheme& theme) {
        KeyBadge badge;
        badge.text = combo.toString();
        badge.textColor = theme.text;
        badge.bgColor = theme.bgAlt;
        badge.borderColor = theme.border;
        badge.fontSize = theme.fontSizeSmall;
        return badge;
    }

    // Backward-compatible alias.
    static KeyBadge renderKeyBadge(const KeyCombo& combo, const WhetstoneTheme& theme) {
        return renderKeySymbol(combo, theme);
    }

    // Render a key badge with platform-aware symbols.
    static KeyBadge renderKeyBadgeSymbols(const KeyCombo& combo, const WhetstoneTheme& theme,
                                          bool macOS = false) {
        KeyBadge badge;
        badge.text = combo.toSymbols(macOS);
        badge.textColor = theme.text;
        badge.bgColor = theme.bgAlt;
        badge.borderColor = theme.border;
        badge.fontSize = theme.fontSizeSmall;
        return badge;
    }

    // Create menu item layout with label left, key right
    static MenuItemLayout renderActionWithKey(
        const std::string& label,
        const std::string& actionId,
        const KeybindingRegistry& registry,
        const WhetstoneTheme& theme,
        const std::string& category = "")
    {
        MenuItemLayout item;
        item.label = label;
        item.category = category;
        auto binding = registry.getBinding(actionId);
        if (shouldRender(binding)) {
            item.shortcut = binding.toString();
            item.symbols = binding.toSymbolsAuto();
            item.hasBadge = true;
        } else {
            item.hasBadge = false;
        }
        // Rough overlap heuristic for headless layout validation.
        const int roughTextWidth = static_cast<int>(label.size() + item.symbols.size());
        const int available = static_cast<int>(theme.fontSizeUI * 8.0f);
        item.overlapRisk = roughTextWidth > available;
        return item;
    }

    // Backward-compatible overload without theme.
    static MenuItemLayout renderActionWithKey(
        const std::string& label,
        const std::string& actionId,
        const KeybindingRegistry& registry,
        const std::string& category = "")
    {
        return renderActionWithKey(label, actionId, registry, getDefaultDarkTheme(), category);
    }

    // Render key chord (multi-key sequence like Ctrl+K Ctrl+S).
    static std::string renderKeyChord(const std::vector<KeyCombo>& combos) {
        std::string result;
        for (size_t i = 0; i < combos.size(); ++i) {
            if (i > 0) result += " ";
            result += combos[i].toString();
        }
        return result;
    }

    // Render key chord with symbols
    static std::string renderKeyChordSymbols(const std::vector<KeyCombo>& combos,
                                             bool macOS = false) {
        std::string result;
        for (size_t i = 0; i < combos.size(); ++i) {
            if (i > 0) result += " ";
            result += combos[i].toSymbols(macOS);
        }
        return result;
    }

    // Check if a binding should render (non-empty key)
    static bool shouldRender(const KeyCombo& combo) {
        return !combo.key.empty();
    }
};
