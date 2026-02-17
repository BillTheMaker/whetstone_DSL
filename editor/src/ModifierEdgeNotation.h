#pragma once
// Step 519: Modifier-Edge Shortcut Notation

#include <string>
#include <vector>
#include <algorithm>

struct ModifierEdges {
    bool left = false;   // Ctrl
    bool right = false;  // Shift
    bool top = false;    // Super/Win
    bool bottom = false; // Alt
};

struct ShortcutGlyph {
    char key = '?';
    ModifierEdges edges;
    std::string fallbackText;
};

class ModifierEdgeNotation {
public:
    static ModifierEdges fromModifiers(bool ctrl, bool shift,
                                       bool superKey, bool alt) {
        return {ctrl, shift, superKey, alt};
    }

    static ShortcutGlyph buildGlyph(char key,
                                    bool ctrl,
                                    bool shift,
                                    bool superKey,
                                    bool alt,
                                    bool glyphEnabled) {
        ShortcutGlyph g;
        g.key = normalizeKey(key);
        g.edges = fromModifiers(ctrl, shift, superKey, alt);
        g.fallbackText = fallbackLabel(g);
        if (!glyphEnabled) {
            g.edges = {};
        }
        return g;
    }

    static int activeEdgeCount(const ModifierEdges& e) {
        return (e.left ? 1 : 0) + (e.right ? 1 : 0) +
               (e.top ? 1 : 0) + (e.bottom ? 1 : 0);
    }

    static bool compactEnough(const ShortcutGlyph& g) {
        // Key plus edge markers should fit compact control regions.
        return g.key != '?' && g.fallbackText.size() <= 20;
    }

    static std::string fallbackLabel(const ShortcutGlyph& g) {
        std::vector<std::string> parts;
        if (g.edges.left) parts.push_back("Ctrl");
        if (g.edges.right) parts.push_back("Shift");
        if (g.edges.top) parts.push_back("Super");
        if (g.edges.bottom) parts.push_back("Alt");
        std::string out;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) out += "+";
            out += parts[i];
        }
        if (!out.empty()) out += "+";
        out += std::string(1, g.key);
        return out;
    }

private:
    static char normalizeKey(char key) {
        if (key >= 'a' && key <= 'z') return static_cast<char>(key - 'a' + 'A');
        if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) return key;
        return '?';
    }
};
