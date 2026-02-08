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
    MOD_NONE  = 0,
    MOD_CTRL  = 1 << 0,
    MOD_SHIFT = 1 << 1,
    MOD_ALT   = 1 << 2,
    MOD_SUPER = 1 << 3  // Cmd on Mac, Win on Windows
};

struct KeyCombo {
    int key;          // virtual key or character code (e.g. 'S', 'Z', 'F')
    int modifiers;    // bitmask of KeyMod

    bool operator==(const KeyCombo& other) const {
        return key == other.key && modifiers == other.modifiers;
    }

    std::string toString() const {
        std::string result;
        if (modifiers & MOD_CTRL)  result += "Ctrl+";
        if (modifiers & MOD_SHIFT) result += "Shift+";
        if (modifiers & MOD_ALT)   result += "Alt+";
        if (modifiers & MOD_SUPER) result += "Super+";
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
        return {0, MOD_NONE};
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
        bind("file.save",           'S', MOD_CTRL);
        bind("file.saveAs",         'S', MOD_CTRL | MOD_SHIFT);
        bind("file.open",           'O', MOD_CTRL);
        bind("file.new",            'N', MOD_CTRL);
        bind("file.close",          'W', MOD_CTRL);

        // Edit operations
        bind("edit.undo",           'Z', MOD_CTRL);
        bind("edit.redo",           'Y', MOD_CTRL);
        bind("edit.cut",            'X', MOD_CTRL);
        bind("edit.copy",           'C', MOD_CTRL);
        bind("edit.paste",          'V', MOD_CTRL);
        bind("edit.selectAll",      'A', MOD_CTRL);
        bind("edit.delete",         127, MOD_NONE); // Delete key
        bind("edit.duplicateLine",  'D', MOD_CTRL | MOD_SHIFT);

        // Search
        bind("search.find",         'F', MOD_CTRL);
        bind("search.replace",      'H', MOD_CTRL);
        bind("search.findNext",     'G', MOD_CTRL);
        bind("search.findPrev",     'G', MOD_CTRL | MOD_SHIFT);
        bind("search.findInFiles",  'F', MOD_CTRL | MOD_SHIFT);

        // Navigation
        bind("nav.goToLine",        'G', MOD_CTRL);
        bind("nav.goToFile",        'P', MOD_CTRL);
        bind("nav.goToSymbol",      'O', MOD_CTRL | MOD_SHIFT);

        // View
        bind("view.toggleSidebar",  'B', MOD_CTRL);
        bind("view.toggleTerminal", '`', MOD_CTRL);
        bind("view.zoomIn",         '=', MOD_CTRL);
        bind("view.zoomOut",        '-', MOD_CTRL);
        bind("view.commandPalette", 'P', MOD_CTRL | MOD_SHIFT);

        // Code
        bind("code.comment",        '/', MOD_CTRL);
        bind("code.format",         'F', MOD_CTRL | MOD_SHIFT | MOD_ALT);
        bind("code.goToDefinition",  0x0D, MOD_CTRL); // Ctrl+Enter or F12
        bind("code.rename",         'R', MOD_CTRL | MOD_SHIFT);

        // Build
        bind("build.run",           'B', MOD_CTRL | MOD_SHIFT);
        bind("build.build",         'B', MOD_CTRL);
    }

    // --- JetBrains profile ---
    void loadJetBrains() {
        // File operations
        bind("file.save",           'S', MOD_CTRL);
        bind("file.saveAs",         'S', MOD_CTRL | MOD_SHIFT);
        bind("file.open",           'O', MOD_CTRL);
        bind("file.new",            'N', MOD_CTRL | MOD_ALT);
        bind("file.close",          'W', MOD_CTRL);

        // Edit operations
        bind("edit.undo",           'Z', MOD_CTRL);
        bind("edit.redo",           'Z', MOD_CTRL | MOD_SHIFT);
        bind("edit.cut",            'X', MOD_CTRL);
        bind("edit.copy",           'C', MOD_CTRL);
        bind("edit.paste",          'V', MOD_CTRL);
        bind("edit.selectAll",      'A', MOD_CTRL);
        bind("edit.delete",         127, MOD_NONE);
        bind("edit.duplicateLine",  'D', MOD_CTRL);

        // Search
        bind("search.find",         'F', MOD_CTRL);
        bind("search.replace",      'R', MOD_CTRL);
        bind("search.findNext",     'L', MOD_CTRL);
        bind("search.findPrev",     'L', MOD_CTRL | MOD_SHIFT);
        bind("search.findInFiles",  'F', MOD_CTRL | MOD_SHIFT);

        // Navigation
        bind("nav.goToLine",        'G', MOD_CTRL);
        bind("nav.goToFile",        'N', MOD_CTRL | MOD_SHIFT);
        bind("nav.goToSymbol",      'O', MOD_CTRL | MOD_ALT | MOD_SHIFT);

        // View
        bind("view.toggleSidebar",  '1', MOD_ALT);
        bind("view.toggleTerminal", '4', MOD_ALT);
        bind("view.zoomIn",         '=', MOD_CTRL);
        bind("view.zoomOut",        '-', MOD_CTRL);
        bind("view.commandPalette", 'A', MOD_CTRL | MOD_SHIFT);

        // Code
        bind("code.comment",        '/', MOD_CTRL);
        bind("code.format",         'L', MOD_CTRL | MOD_ALT);
        bind("code.goToDefinition",  'B', MOD_CTRL);
        bind("code.rename",         'R', MOD_SHIFT | MOD_ALT);

        // Build
        bind("build.run",           'R', MOD_CTRL | MOD_SHIFT);
        bind("build.build",         'B', MOD_CTRL | MOD_SHIFT);
    }

    // --- Emacs profile ---
    void loadEmacs() {
        // File operations
        bind("file.save",           'S', MOD_CTRL);
        bind("file.saveAs",         'S', MOD_CTRL | MOD_SHIFT);
        bind("file.open",           'O', MOD_CTRL);
        bind("file.new",            'N', MOD_CTRL);
        bind("file.close",          'W', MOD_CTRL);

        // Edit operations
        bind("edit.undo",           'Z', MOD_CTRL);
        bind("edit.redo",           'Z', MOD_CTRL | MOD_SHIFT);
        bind("edit.cut",            'W', MOD_CTRL);
        bind("edit.copy",           'W', MOD_ALT);
        bind("edit.paste",          'Y', MOD_CTRL);
        bind("edit.selectAll",      'A', MOD_CTRL);
        bind("edit.delete",         'D', MOD_CTRL);
        bind("edit.duplicateLine",  'D', MOD_CTRL | MOD_SHIFT);

        // Search
        bind("search.find",         'S', MOD_CTRL);
        bind("search.replace",      'R', MOD_ALT);
        bind("search.findNext",     'S', MOD_CTRL);
        bind("search.findPrev",     'R', MOD_CTRL);
        bind("search.findInFiles",  'F', MOD_CTRL | MOD_SHIFT);

        // Navigation
        bind("nav.goToLine",        'G', MOD_ALT);
        bind("nav.goToFile",        'F', MOD_CTRL);
        bind("nav.goToSymbol",      'O', MOD_CTRL | MOD_SHIFT);

        // View
        bind("view.toggleSidebar",  'B', MOD_CTRL);
        bind("view.toggleTerminal", '`', MOD_CTRL);
        bind("view.zoomIn",         '=', MOD_CTRL);
        bind("view.zoomOut",        '-', MOD_CTRL);
        bind("view.commandPalette", 'X', MOD_ALT);

        // Code
        bind("code.comment",        ';', MOD_ALT);
        bind("code.format",         'F', MOD_CTRL | MOD_ALT);
        bind("code.goToDefinition",  '.', MOD_ALT);
        bind("code.rename",         'R', MOD_CTRL | MOD_SHIFT);

        // Build
        bind("build.run",           'C', MOD_CTRL | MOD_SHIFT);
        bind("build.build",         'B', MOD_CTRL | MOD_SHIFT);
    }

    KeybindingProfile profile_;
    std::map<std::string, KeyCombo> bindings_;
};
