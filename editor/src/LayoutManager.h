// LayoutManager.h — Docking layout presets (Step 76)
//
// Defines preset layouts (VSCode, Emacs, JetBrains) with panel sizes
// and dock configuration.  The actual ImGui DockBuilder calls happen in
// main.cpp; LayoutManager provides the declarative data.
//
// Presets:
//   VSCode     — Explorer 20% left, Editor 60% center, Panel 20% bottom
//   Emacs      — Full-frame editor, optional side panels, minibuffer bar
//   JetBrains  — Project tree 20% left, Editor center, tool windows right+bottom

#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>

// ---- Enums ----------------------------------------------------------------

enum class LayoutPreset { VSCode, Emacs, JetBrains };

enum class DockDirection { Left, Right, Bottom, Top, Center };

// ---- Data types -----------------------------------------------------------

struct DockPanel {
    std::string   id;          // ImGui window name ("Explorer", "Editor", "Panel", etc.)
    DockDirection direction;   // Where this panel docks relative to the main area
    float         sizeRatio;   // Fraction of the split axis (0.0 – 1.0)
    bool          visible;     // Starts visible?
};

struct LayoutConfig {
    LayoutPreset           preset;
    std::string            name;
    std::string            description;
    std::vector<DockPanel> panels;
    bool                   showStatusBar = true;
};

// ---- LayoutManager --------------------------------------------------------

class LayoutManager {
    LayoutPreset                       currentPreset_ = LayoutPreset::VSCode;
    std::map<LayoutPreset, LayoutConfig> presets_;
    bool                               layoutDirty_   = true;

public:
    LayoutManager() { initPresets(); }

    // --- Preset access ------------------------------------------------------

    static std::vector<LayoutPreset> availablePresets() {
        return { LayoutPreset::VSCode, LayoutPreset::Emacs, LayoutPreset::JetBrains };
    }

    static const char* presetName(LayoutPreset p) {
        switch (p) {
            case LayoutPreset::VSCode:    return "VSCode";
            case LayoutPreset::Emacs:     return "Emacs";
            case LayoutPreset::JetBrains: return "JetBrains";
        }
        return "Unknown";
    }

    static LayoutPreset presetFromName(const std::string& name) {
        if (name == "Emacs")     return LayoutPreset::Emacs;
        if (name == "JetBrains") return LayoutPreset::JetBrains;
        return LayoutPreset::VSCode; // default
    }

    // --- Current preset -----------------------------------------------------

    LayoutPreset getPreset() const { return currentPreset_; }

    void setPreset(LayoutPreset p) {
        if (currentPreset_ != p) {
            currentPreset_ = p;
            layoutDirty_ = true;
        }
    }

    bool isLayoutDirty() const { return layoutDirty_; }
    void clearDirty() { layoutDirty_ = false; }
    void markDirty()  { layoutDirty_ = true; }

    const LayoutConfig& getConfig() const {
        return presets_.at(currentPreset_);
    }

    const LayoutConfig& getConfig(LayoutPreset p) const {
        return presets_.at(p);
    }

    // --- Panel queries ------------------------------------------------------

    const DockPanel* findPanel(const std::string& panelId) const {
        const auto& cfg = getConfig();
        for (const auto& p : cfg.panels) {
            if (p.id == panelId) return &p;
        }
        return nullptr;
    }

    bool isPanelVisible(const std::string& panelId) const {
        const auto* p = findPanel(panelId);
        return p ? p->visible : false;
    }

    std::vector<std::string> getVisiblePanelIds() const {
        std::vector<std::string> ids;
        const auto& cfg = getConfig();
        for (const auto& p : cfg.panels) {
            if (p.visible) ids.push_back(p.id);
        }
        return ids;
    }

    float getPanelRatio(const std::string& panelId) const {
        const auto* p = findPanel(panelId);
        return p ? p->sizeRatio : 0.0f;
    }

    // --- Persistence (simple key=value) -------------------------------------

    bool saveToFile(const std::string& path) const {
        std::ofstream f(path);
        if (!f.is_open()) return false;
        f << "layout=" << presetName(currentPreset_) << "\n";
        return true;
    }

    bool loadFromFile(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::string line;
        while (std::getline(f, line)) {
            if (line.substr(0, 7) == "layout=") {
                setPreset(presetFromName(line.substr(7)));
                return true;
            }
        }
        return false;
    }

private:
    void initPresets() {
        // ---- VSCode --------------------------------------------------------
        LayoutConfig vscode;
        vscode.preset      = LayoutPreset::VSCode;
        vscode.name        = "VSCode";
        vscode.description = "Explorer 20% left, Editor center, Panel 20% bottom, status bar";
        vscode.showStatusBar = true;
        vscode.panels = {
            { "Explorer",     DockDirection::Left,   0.20f, true  },
            { "Editor",       DockDirection::Center, 0.60f, true  },
            { "Panel",        DockDirection::Bottom, 0.20f, true  },
        };
        presets_[LayoutPreset::VSCode] = std::move(vscode);

        // ---- Emacs ---------------------------------------------------------
        LayoutConfig emacs;
        emacs.preset      = LayoutPreset::Emacs;
        emacs.name        = "Emacs";
        emacs.description = "Full-frame editor, optional side panels, minibuffer bottom bar";
        emacs.showStatusBar = true;
        emacs.panels = {
            { "Explorer",     DockDirection::Left,   0.15f, false },
            { "Editor",       DockDirection::Center, 0.85f, true  },
            { "Minibuffer",   DockDirection::Bottom, 0.04f, true  },
            { "Panel",        DockDirection::Bottom, 0.15f, false },
        };
        presets_[LayoutPreset::Emacs] = std::move(emacs);

        // ---- JetBrains -----------------------------------------------------
        LayoutConfig jb;
        jb.preset      = LayoutPreset::JetBrains;
        jb.name        = "JetBrains";
        jb.description = "Project tree left, Editor center, tool windows right and bottom";
        jb.showStatusBar = true;
        jb.panels = {
            { "Explorer",     DockDirection::Left,   0.20f, true  },
            { "Editor",       DockDirection::Center, 0.50f, true  },
            { "ToolWindow",   DockDirection::Right,  0.15f, true  },
            { "Panel",        DockDirection::Bottom, 0.25f, true  },
        };
        presets_[LayoutPreset::JetBrains] = std::move(jb);
    }
};
