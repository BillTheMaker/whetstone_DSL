#pragma once
// Step 54: Configurable keybinding profiles
//
// KeybindingManager provides keybinding lookup with swappable profiles.
// Profiles: VSCode (default), JetBrains, Emacs.
// Each profile maps action names to key combinations.
// The editor queries this at runtime to dispatch key events.

#include <string>
#include <map>
#include <vector>

// Modifier flags (bitmask)
enum KeyMod : int {
    WMOD_NONE  = 0,
    WMOD_CTRL  = 1 << 0,
    WMOD_SHIFT = 1 << 1,
    WMOD_ALT   = 1 << 2,
    WMOD_SUPER = 1 << 3  // Cmd on Mac, Win on Windows
};

struct KeyCombo {
    int key;          // virtual key or character code (e.g. 'S', 'Z', 'F')
    int modifiers;    // bitmask of KeyMod

    bool operator==(const KeyCombo& other) const {
        return key == other.key && modifiers == other.modifiers;
    }

    std::string toString() const {
        std::string result;
        if (modifiers & WMOD_CTRL)  result += "Ctrl+";
        if (modifiers & WMOD_SHIFT) result += "Shift+";
        if (modifiers & WMOD_ALT)   result += "Alt+";
        if (modifiers & WMOD_SUPER) result += "Super+";
        if (key >= 'A' && key <= 'Z') {
            result += (char)key;
        } else if (key >= '0' && key <= '9') {
            result += (char)key;
        } else {
            result += "[" + std::to_string(key) + "]";
        }
        return result;
    }
};

enum class KeybindingProfile {
    VSCode,
    JetBrains,
    Emacs
};

class KeybindingManager {
public:
    KeybindingManager() {
        setProfile(KeybindingProfile::VSCode);
    }

    void setProfile(KeybindingProfile profile) {
        profile_ = profile;
        bindings_.clear();
        switch (profile) {
            case KeybindingProfile::VSCode:    loadVSCode();    break;
            case KeybindingProfile::JetBrains: loadJetBrains(); break;
            case KeybindingProfile::Emacs:     loadEmacs();     break;
        }
    }

    KeybindingProfile getProfile() const { return profile_; }

    static const char* profileName(KeybindingProfile p) {
        switch (p) {
            case KeybindingProfile::VSCode:    return "VSCode";
            case KeybindingProfile::JetBrains: return "JetBrains";
            case KeybindingProfile::Emacs:     return "Emacs";
        }
        return "Unknown";
    }

    static KeybindingProfile profileFromName(const std::string& name) {
        if (name == "VSCode" || name == "vscode")       return KeybindingProfile::VSCode;
        if (name == "JetBrains" || name == "jetbrains") return KeybindingProfile::JetBrains;
        if (name == "Emacs" || name == "emacs")         return KeybindingProfile::Emacs;
        return KeybindingProfile::VSCode; // default
    }

    // Look up the key combo for an action (returns empty combo if not bound)
    KeyCombo getBinding(const std::string& action) const {
        auto it = bindings_.find(action);
        if (it != bindings_.end()) return it->second;
        return {0, WMOD_NONE};
    }

    // Look up the action for a key combo (returns "" if not bound)
    std::string getAction(const KeyCombo& combo) const {
        for (const auto& [action, binding] : bindings_) {
            if (binding == combo) return action;
        }
        return "";
    }

    // Override a single binding
    void setBinding(const std::string& action, KeyCombo combo) {
        bindings_[action] = combo;
    }

    // List all actions with their bindings
    std::vector<std::string> listActions() const {
        std::vector<std::string> actions;
        for (const auto& [action, _] : bindings_) {
            actions.push_back(action);
        }
        return actions;
    }

    // Available profiles
    static std::vector<KeybindingProfile> availableProfiles() {
        return {KeybindingProfile::VSCode, KeybindingProfile::JetBrains,
                KeybindingProfile::Emacs};
    }

private:
    void bind(const std::string& action, int key, int mods) {
        bindings_[action] = {key, mods};
    }

    // --- VSCode profile (default) ---
    void loadVSCode() {
        // File operations
        bind("file.save",           'S', WMOD_CTRL);
        bind("file.saveAs",         'S', WMOD_CTRL | WMOD_SHIFT);
        bind("file.open",           'O', WMOD_CTRL);
        bind("file.new",            'N', WMOD_CTRL);
        bind("file.close",          'W', WMOD_CTRL);

        // Edit operations
        bind("edit.undo",           'Z', WMOD_CTRL);
        bind("edit.redo",           'Y', WMOD_CTRL);
        bind("edit.cut",            'X', WMOD_CTRL);
        bind("edit.copy",           'C', WMOD_CTRL);
        bind("edit.paste",          'V', WMOD_CTRL);
        bind("edit.selectAll",      'A', WMOD_CTRL);
        bind("edit.delete",         127, WMOD_NONE); // Delete key
        bind("edit.duplicateLine",  'D', WMOD_CTRL | WMOD_SHIFT);

        // Search
        bind("search.find",         'F', WMOD_CTRL);
        bind("search.replace",      'H', WMOD_CTRL);
        bind("search.findNext",     'G', WMOD_CTRL);
        bind("search.findPrev",     'G', WMOD_CTRL | WMOD_SHIFT);
        bind("search.findInFiles",  'F', WMOD_CTRL | WMOD_SHIFT);

        // Navigation
        bind("nav.goToLine",        'G', WMOD_CTRL);
        bind("nav.goToFile",        'P', WMOD_CTRL);
        bind("nav.goToSymbol",      'O', WMOD_CTRL | WMOD_SHIFT);

        // View
        bind("view.toggleSidebar",  'B', WMOD_CTRL);
        bind("view.toggleTerminal", '`', WMOD_CTRL);
        bind("view.zoomIn",         '=', WMOD_CTRL);
        bind("view.zoomOut",        '-', WMOD_CTRL);
        bind("view.commandPalette", 'P', WMOD_CTRL | WMOD_SHIFT);

        // Code
        bind("code.comment",        '/', WMOD_CTRL);
        bind("code.format",         'F', WMOD_CTRL | WMOD_SHIFT | WMOD_ALT);
        bind("code.goToDefinition",  0x0D, WMOD_CTRL); // Ctrl+Enter or F12
        bind("code.rename",         'R', WMOD_CTRL | WMOD_SHIFT);

        // Build
        bind("build.run",           'B', WMOD_CTRL | WMOD_SHIFT);
        bind("build.build",         'B', WMOD_CTRL);
    }

    // --- JetBrains profile ---
    void loadJetBrains() {
        // File operations
        bind("file.save",           'S', WMOD_CTRL);
        bind("file.saveAs",         'S', WMOD_CTRL | WMOD_SHIFT);
        bind("file.open",           'O', WMOD_CTRL);
        bind("file.new",            'N', WMOD_CTRL | WMOD_ALT);
        bind("file.close",          'W', WMOD_CTRL);

        // Edit operations
        bind("edit.undo",           'Z', WMOD_CTRL);
        bind("edit.redo",           'Z', WMOD_CTRL | WMOD_SHIFT);
        bind("edit.cut",            'X', WMOD_CTRL);
        bind("edit.copy",           'C', WMOD_CTRL);
        bind("edit.paste",          'V', WMOD_CTRL);
        bind("edit.selectAll",      'A', WMOD_CTRL);
        bind("edit.delete",         127, WMOD_NONE);
        bind("edit.duplicateLine",  'D', WMOD_CTRL);

        // Search
        bind("search.find",         'F', WMOD_CTRL);
        bind("search.replace",      'R', WMOD_CTRL);
        bind("search.findNext",     'L', WMOD_CTRL);
        bind("search.findPrev",     'L', WMOD_CTRL | WMOD_SHIFT);
        bind("search.findInFiles",  'F', WMOD_CTRL | WMOD_SHIFT);

        // Navigation
        bind("nav.goToLine",        'G', WMOD_CTRL);
        bind("nav.goToFile",        'N', WMOD_CTRL | WMOD_SHIFT);
        bind("nav.goToSymbol",      'O', WMOD_CTRL | WMOD_ALT | WMOD_SHIFT);

        // View
        bind("view.toggleSidebar",  '1', WMOD_ALT);
        bind("view.toggleTerminal", '4', WMOD_ALT);
        bind("view.zoomIn",         '=', WMOD_CTRL);
        bind("view.zoomOut",        '-', WMOD_CTRL);
        bind("view.commandPalette", 'A', WMOD_CTRL | WMOD_SHIFT);

        // Code
        bind("code.comment",        '/', WMOD_CTRL);
        bind("code.format",         'L', WMOD_CTRL | WMOD_ALT);
        bind("code.goToDefinition",  'B', WMOD_CTRL);
        bind("code.rename",         'R', WMOD_SHIFT | WMOD_ALT);

        // Build
        bind("build.run",           'R', WMOD_CTRL | WMOD_SHIFT);
        bind("build.build",         'B', WMOD_CTRL | WMOD_SHIFT);
    }

    // --- Emacs profile ---
    void loadEmacs() {
        // File operations
        bind("file.save",           'S', WMOD_CTRL);
        bind("file.saveAs",         'S', WMOD_CTRL | WMOD_SHIFT);
        bind("file.open",           'O', WMOD_CTRL);
        bind("file.new",            'N', WMOD_CTRL);
        bind("file.close",          'W', WMOD_CTRL);

        // Edit operations
        bind("edit.undo",           'Z', WMOD_CTRL);
        bind("edit.redo",           'Z', WMOD_CTRL | WMOD_SHIFT);
        bind("edit.cut",            'W', WMOD_CTRL);
        bind("edit.copy",           'W', WMOD_ALT);
        bind("edit.paste",          'Y', WMOD_CTRL);
        bind("edit.selectAll",      'A', WMOD_CTRL);
        bind("edit.delete",         'D', WMOD_CTRL);
        bind("edit.duplicateLine",  'D', WMOD_CTRL | WMOD_SHIFT);

        // Search
        bind("search.find",         'S', WMOD_CTRL);
        bind("search.replace",      'R', WMOD_ALT);
        bind("search.findNext",     'S', WMOD_CTRL);
        bind("search.findPrev",     'R', WMOD_CTRL);
        bind("search.findInFiles",  'F', WMOD_CTRL | WMOD_SHIFT);

        // Navigation
        bind("nav.goToLine",        'G', WMOD_ALT);
        bind("nav.goToFile",        'F', WMOD_CTRL);
        bind("nav.goToSymbol",      'O', WMOD_CTRL | WMOD_SHIFT);

        // View
        bind("view.toggleSidebar",  'B', WMOD_CTRL);
        bind("view.toggleTerminal", '`', WMOD_CTRL);
        bind("view.zoomIn",         '=', WMOD_CTRL);
        bind("view.zoomOut",        '-', WMOD_CTRL);
        bind("view.commandPalette", 'X', WMOD_ALT);

        // Code
        bind("code.comment",        ';', WMOD_ALT);
        bind("code.format",         'F', WMOD_CTRL | WMOD_ALT);
        bind("code.goToDefinition",  '.', WMOD_ALT);
        bind("code.rename",         'R', WMOD_CTRL | WMOD_SHIFT);

        // Build
        bind("build.run",           'C', WMOD_CTRL | WMOD_SHIFT);
        bind("build.build",         'B', WMOD_CTRL | WMOD_SHIFT);
    }

    KeybindingProfile profile_;
    std::map<std::string, KeyCombo> bindings_;
};
