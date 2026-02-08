#pragma once
// Step 55: Welcome screen component
//
// WelcomeScreen provides a VSCode-style welcome tab for the editor.
// Shows recent files, quick actions, keybinding profile info, and tips.
// Rendered as an ImGui panel or tab content.

#include <string>
#include <vector>
#include <functional>

struct WelcomeAction {
    std::string label;
    std::string description;
    std::string shortcut;  // e.g. "Ctrl+N"
};

struct RecentFile {
    std::string path;
    std::string language;
    std::string displayName;  // just the filename
};

class WelcomeScreen {
public:
    WelcomeScreen() {
        // Default quick actions
        actions_ = {
            {"New File",        "Create a new empty file",               "Ctrl+N"},
            {"Open File",       "Open an existing file",                 "Ctrl+O"},
            {"Change Language", "Switch the active language mode",       ""},
            {"Keybindings",     "Choose VSCode, JetBrains, or Emacs keys", ""},
        };

        tips_ = {
            "Use the Language menu to switch between Python, C++, and Elisp.",
            "The AST tab shows your code's abstract syntax tree in real time.",
            "The Generated tab shows target-language output from the AST.",
            "Switch keybinding profiles from the Keybindings menu.",
            "Whetstone annotations (@Reclaim, @Owner, @Lifetime) express memory intent.",
            "The Highlighted tab shows syntax coloring via tree-sitter.",
        };
    }

    // --- Data access ---

    const std::string& getTitle() const { return title_; }
    void setTitle(const std::string& t) { title_ = t; }

    const std::string& getSubtitle() const { return subtitle_; }
    void setSubtitle(const std::string& s) { subtitle_ = s; }

    const std::vector<WelcomeAction>& getActions() const { return actions_; }
    void setActions(const std::vector<WelcomeAction>& a) { actions_ = a; }

    const std::vector<RecentFile>& getRecentFiles() const { return recentFiles_; }
    void addRecentFile(const std::string& path, const std::string& lang) {
        // Extract display name from path
        std::string name = path;
        auto sep = path.find_last_of("/\\");
        if (sep != std::string::npos) name = path.substr(sep + 1);
        // Don't add duplicates
        for (const auto& f : recentFiles_) {
            if (f.path == path) return;
        }
        // Keep max 10 recent files
        if (recentFiles_.size() >= 10) recentFiles_.pop_back();
        recentFiles_.insert(recentFiles_.begin(), {path, lang, name});
    }

    void clearRecentFiles() { recentFiles_.clear(); }

    const std::vector<std::string>& getTips() const { return tips_; }

    // Get the next tip (cycles through)
    std::string getNextTip() {
        if (tips_.empty()) return "";
        std::string tip = tips_[tipIndex_ % tips_.size()];
        ++tipIndex_;
        return tip;
    }

    // Get a specific tip by index
    std::string getTip(size_t index) const {
        if (index >= tips_.size()) return "";
        return tips_[index];
    }

    // --- Visibility ---

    bool isVisible() const { return visible_; }
    void setVisible(bool v) { visible_ = v; }
    void show() { visible_ = true; }
    void hide() { visible_ = false; }

    // --- Version info ---

    const std::string& getVersion() const { return version_; }
    void setVersion(const std::string& v) { version_ = v; }

private:
    std::string title_    = "Whetstone Editor";
    std::string subtitle_ = "Semantic Annotation DSL for Cross-Language Code Generation";
    std::string version_  = "0.3.0";
    bool visible_         = true;

    std::vector<WelcomeAction> actions_;
    std::vector<RecentFile>    recentFiles_;
    std::vector<std::string>   tips_;
    size_t tipIndex_ = 0;
};
