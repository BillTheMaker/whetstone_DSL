#pragma once
// Step 343-351: Keybinding registry — centralized action→key combo mapping
// Headless data model: no ImGui dependency, supports symbol rendering

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

enum class KeyModifier : uint8_t {
    None  = 0,
    Ctrl  = 1 << 0,
    Alt   = 1 << 1,
    Shift = 1 << 2,
    Super = 1 << 3
};

inline uint8_t operator|(KeyModifier a, KeyModifier b) {
    return static_cast<uint8_t>(a) | static_cast<uint8_t>(b);
}

inline bool hasModifier(uint8_t mods, KeyModifier m) {
    return (mods & static_cast<uint8_t>(m)) != 0;
}

struct KeyCombo {
    uint8_t modifiers = 0;
    std::string key;  // "S", "P", "Z", "F5", "F8", "Enter", "Escape", etc.

    bool empty() const { return key.empty(); }

    std::string toString() const {
        std::string result;
        if (hasModifier(modifiers, KeyModifier::Ctrl)) result += "Ctrl+";
        if (hasModifier(modifiers, KeyModifier::Alt)) result += "Alt+";
        if (hasModifier(modifiers, KeyModifier::Shift)) result += "Shift+";
        if (hasModifier(modifiers, KeyModifier::Super)) result += "Super+";
        result += key;
        return result;
    }

    // Platform-aware symbols (macOS style vs Linux/Windows)
    std::string toSymbols(bool macOS = false) const {
        std::string result;
        if (macOS) {
            if (hasModifier(modifiers, KeyModifier::Ctrl)) result += "\xE2\x8C\x83"; // ⌃
            if (hasModifier(modifiers, KeyModifier::Alt)) result += "\xE2\x8C\xA5";   // ⌥
            if (hasModifier(modifiers, KeyModifier::Shift)) result += "\xE2\x87\xA7"; // ⇧
            if (hasModifier(modifiers, KeyModifier::Super)) result += "\xE2\x8C\x98"; // ⌘
            result += keyToSymbol(key);
        } else {
            result = toString(); // Linux/Windows uses text form
        }
        return result;
    }

    static std::string normalizeKey(const std::string& in) {
        std::string k = in;
        std::transform(k.begin(), k.end(), k.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return k;
    }

    static std::string keyToSymbol(const std::string& keyName) {
        std::string k = normalizeKey(keyName);
        if (k == "ENTER" || k == "RETURN") return "\xE2\x86\xB5"; // ↵
        if (k == "ESC" || k == "ESCAPE") return "\xE2\x8E\x8B";   // ⎋
        if (k == "TAB") return "\xE2\x87\xA5";                    // ⇥
        if (k == "SPACE") return "\xE2\x90\xA0";                  // ␠
        if (k == "BACKSPACE") return "\xE2\x8C\xAB";              // ⌫
        if (k == "DELETE") return "\xE2\x8C\xA6";                 // ⌦
        return keyName;
    }

    std::string toSymbolsAuto() const {
#ifdef __APPLE__
        return toSymbols(true);
#else
        return toSymbols(false);
#endif
    }

    bool operator==(const KeyCombo& other) const {
        return modifiers == other.modifiers && key == other.key;
    }

    nlohmann::json toJson() const {
        return {{"modifiers", modifiers}, {"key", key}};
    }

    static KeyCombo fromJson(const nlohmann::json& j) {
        KeyCombo c;
        c.modifiers = j.value("modifiers", 0);
        c.key = j.value("key", "");
        return c;
    }

    // Helper constructors
    static KeyCombo ctrl(const std::string& k) {
        return {static_cast<uint8_t>(KeyModifier::Ctrl), k};
    }
    static KeyCombo ctrlShift(const std::string& k) {
        return {KeyModifier::Ctrl | KeyModifier::Shift, k};
    }
    static KeyCombo alt(const std::string& k) {
        return {static_cast<uint8_t>(KeyModifier::Alt), k};
    }
    static KeyCombo fkey(const std::string& k) {
        return {0, k};
    }
    static KeyCombo shiftFkey(const std::string& k) {
        return {static_cast<uint8_t>(KeyModifier::Shift), k};
    }

    struct InputState {
        bool ctrl = false;
        bool alt = false;
        bool shift = false;
        bool super = false;
        std::string key;
    };

    bool matches(const InputState& input) const {
        if (normalizeKey(key) != normalizeKey(input.key)) return false;
        if (hasModifier(modifiers, KeyModifier::Ctrl) != input.ctrl) return false;
        if (hasModifier(modifiers, KeyModifier::Alt) != input.alt) return false;
        if (hasModifier(modifiers, KeyModifier::Shift) != input.shift) return false;
        if (hasModifier(modifiers, KeyModifier::Super) != input.super) return false;
        return true;
    }
};

struct KeyAction {
    std::string id;          // "save-buffer", "toggle-ast-panel", etc.
    std::string label;       // "Save Buffer", "Toggle AST Panel"
    std::string category;    // "File", "Edit", "View", "Tools", "Workflow"
    std::string description; // optional longer description
};

class KeybindingRegistry {
public:
    KeybindingRegistry() = default;

    void registerAction(const std::string& id, const std::string& label,
                        const std::string& category, const std::string& description = "") {
        actions_[id] = {id, label, category, description};
    }

    void bind(const std::string& actionId, const KeyCombo& combo) {
        bindings_[actionId] = combo;
    }

    KeyCombo getBinding(const std::string& actionId) const {
        auto it = bindings_.find(actionId);
        return it != bindings_.end() ? it->second : KeyCombo{};
    }

    std::string getSymbols(const std::string& actionId, bool macOS = false) const {
        auto combo = getBinding(actionId);
        return combo.empty() ? "" : combo.toSymbols(macOS);
    }

    std::optional<std::string> findConflict(const std::string& actionId, const KeyCombo& combo) const {
        for (const auto& [aid, c] : bindings_) {
            if (aid != actionId && c == combo) return aid;
        }
        return std::nullopt;
    }

    std::vector<KeyAction> getActions() const {
        std::vector<KeyAction> result;
        for (const auto& [id, action] : actions_) result.push_back(action);
        return result;
    }

    std::vector<KeyAction> getActionsByCategory(const std::string& category) const {
        std::vector<KeyAction> result;
        for (const auto& [id, action] : actions_) {
            if (action.category == category) result.push_back(action);
        }
        return result;
    }

    std::map<std::string, KeyCombo> getAllBindings() const { return bindings_; }
    std::map<std::string, KeyCombo> getAll() const { return bindings_; }

    int actionCount() const { return static_cast<int>(actions_.size()); }
    int bindingCount() const { return static_cast<int>(bindings_.size()); }

    void loadDefaults() {
        // File actions
        registerAction("save-buffer", "Save Buffer", "File");
        registerAction("save-all", "Save All", "File");
        registerAction("open-file", "Open File", "File");
        registerAction("close-buffer", "Close Buffer", "File");

        // Edit actions
        registerAction("undo", "Undo", "Edit");
        registerAction("redo", "Redo", "Edit");
        registerAction("find-in-file", "Find in File", "Edit");
        registerAction("find-in-project", "Find in Project", "Edit");

        // View actions
        registerAction("command-palette", "Command Palette", "View");
        registerAction("keyboard-shortcuts", "Keyboard Shortcuts", "Help");
        registerAction("toggle-file-tree", "Toggle File Tree", "View");
        registerAction("toggle-ast-view", "Toggle AST View", "View");
        registerAction("toggle-diagnostics", "Toggle Diagnostics", "View");
        registerAction("toggle-annotations", "Toggle Annotations", "View");

        // Tools actions
        registerAction("run-pipeline", "Run Pipeline", "Tools");
        registerAction("generate-code", "Generate Code", "Tools");
        registerAction("infer-annotations", "Infer Annotations", "Tools");
        registerAction("validate", "Validate", "Tools");

        // Workflow actions
        registerAction("create-workflow", "Create Workflow", "Workflow");
        registerAction("route-all", "Route All Tasks", "Workflow");

        // Navigation
        registerAction("next-diagnostic", "Next Diagnostic", "Navigation");
        registerAction("prev-diagnostic", "Previous Diagnostic", "Navigation");
        registerAction("go-to-definition", "Go to Definition", "Navigation");
        registerAction("go-to-symbol", "Go to Symbol", "Navigation");

        // Default bindings
        bind("save-buffer", KeyCombo::ctrl("S"));
        bind("save-all", KeyCombo::ctrlShift("S"));
        bind("undo", KeyCombo::ctrl("Z"));
        bind("redo", KeyCombo::ctrlShift("Z"));
        bind("find-in-file", KeyCombo::ctrl("F"));
        bind("find-in-project", KeyCombo::ctrlShift("F"));
        bind("command-palette", KeyCombo::ctrl("P"));
        bind("keyboard-shortcuts", KeyCombo::ctrlShift("?"));
        bind("toggle-file-tree", KeyCombo::ctrl("B"));
        bind("toggle-diagnostics", KeyCombo::ctrlShift("D"));
        bind("run-pipeline", KeyCombo::fkey("F5"));
        bind("next-diagnostic", KeyCombo::fkey("F8"));
        bind("prev-diagnostic", KeyCombo::shiftFkey("F8"));
        bind("go-to-definition", KeyCombo::fkey("F12"));
        bind("go-to-symbol", KeyCombo::ctrlShift("O"));
    }

    nlohmann::json toJson() const {
        nlohmann::json j;
        nlohmann::json bindingsJson;
        for (const auto& [id, combo] : bindings_) {
            bindingsJson[id] = combo.toJson();
        }
        j["bindings"] = bindingsJson;
        return j;
    }

    void loadFromJson(const nlohmann::json& j) {
        if (j.contains("bindings")) {
            for (auto& [id, cj] : j["bindings"].items()) {
                bindings_[id] = KeyCombo::fromJson(cj);
            }
        }
    }

    bool saveToJson(const std::string& path) const {
        std::ofstream out(path);
        if (!out.good()) return false;
        out << toJson().dump(2);
        return out.good();
    }

    bool loadFromJsonFile(const std::string& path) {
        std::ifstream in(path);
        if (!in.good()) return false;
        nlohmann::json j;
        try {
            in >> j;
            loadFromJson(j);
            return true;
        } catch (...) {
            return false;
        }
    }

    std::optional<std::string> processInput(const KeyCombo::InputState& input) const {
        for (const auto& [actionId, combo] : bindings_) {
            if (combo.matches(input)) return actionId;
        }
        return std::nullopt;
    }

private:
    std::map<std::string, KeyAction> actions_;
    std::map<std::string, KeyCombo> bindings_;
};
