#pragma once
// Step 354: Menu Bar with key symbols.
// Headless data model used by tests and UI integration points.

#include <optional>
#include <string>
#include <vector>
#include "KeySymbolRenderer.h"

struct MenuItem {
    std::string id;
    std::string label;
    std::string shortcut;   // text form, e.g. Ctrl+S
    std::string symbols;    // symbol/text depending platform, e.g. ⌃S
    bool isSeparator = false;
    bool enabled = true;
    bool hasSubmenu = false;
    std::vector<MenuItem> submenuItems;

    static MenuItem separator() {
        MenuItem m;
        m.isSeparator = true;
        return m;
    }
};

struct Menu {
    std::string title;
    std::vector<MenuItem> items;
};

class MenuBar {
public:
    void addMenu(const Menu& menu) { menus_.push_back(menu); }

    const std::vector<Menu>& getMenus() const { return menus_; }

    size_t menuCount() const { return menus_.size(); }

    const Menu* getMenu(const std::string& title) const {
        for (const auto& m : menus_) {
            if (m.title == title) return &m;
        }
        return nullptr;
    }

    bool hasMenu(const std::string& title) const {
        return getMenu(title) != nullptr;
    }

    MenuItem findItem(const std::string& id) const {
        for (const auto& menu : menus_) {
            for (const auto& item : menu.items) {
                if (item.id == id) return item;
                if (item.hasSubmenu) {
                    for (const auto& sub : item.submenuItems) {
                        if (sub.id == id) return sub;
                    }
                }
            }
        }
        return {};
    }

    bool hasItem(const std::string& id) const {
        return !findItem(id).id.empty();
    }

    int separatorCount(const std::string& menuTitle) const {
        const Menu* menu = getMenu(menuTitle);
        if (!menu) return 0;
        int count = 0;
        for (const auto& item : menu->items) {
            if (item.isSeparator) count++;
        }
        return count;
    }

    bool hasSubmenu(const std::string& itemId) const {
        for (const auto& menu : menus_) {
            for (const auto& item : menu.items) {
                if (item.id == itemId) return item.hasSubmenu && !item.submenuItems.empty();
            }
        }
        return false;
    }

    void openMenu(const std::string& title) {
        openMenu_ = title;
    }

    bool isMenuOpen(const std::string& title) const {
        return openMenu_ == title;
    }

    bool isAnyMenuOpen() const { return !openMenu_.empty(); }

    std::optional<std::string> selectItem(const std::string& itemId) {
        MenuItem item = findItem(itemId);
        if (item.id.empty() || !item.enabled || item.isSeparator) return std::nullopt;
        openMenu_.clear(); // selecting an action closes menus
        return item.id;
    }

    static MenuBar buildDefaults(const KeybindingRegistry& reg,
                                 const WhetstoneTheme& theme = getDefaultDarkTheme()) {
        MenuBar bar;

        auto makeAction = [&](const std::string& id, const std::string& fallback) -> MenuItem {
            MenuItem item;
            item.id = id;
            item.label = fallback;

            for (const auto& action : reg.getActions()) {
                if (action.id == id) {
                    item.label = action.label;
                    break;
                }
            }

            auto layout = KeySymbolRenderer::renderActionWithKey(item.label, id, reg, theme);
            item.shortcut = layout.shortcut;
            item.symbols = layout.symbols;
            return item;
        };

        // File
        {
            Menu m;
            m.title = "File";
            m.items.push_back(makeAction("new-file", "New"));
            m.items.push_back(makeAction("open-file", "Open File"));
            m.items.push_back(MenuItem::separator());
            m.items.push_back(makeAction("save-buffer", "Save"));
            m.items.push_back(makeAction("save-all", "Save All"));
            m.items.push_back(MenuItem::separator());
            m.items.push_back(makeAction("close-buffer", "Close"));
            bar.addMenu(m);
        }

        // Edit
        {
            Menu m;
            m.title = "Edit";
            m.items.push_back(makeAction("undo", "Undo"));
            m.items.push_back(makeAction("redo", "Redo"));
            m.items.push_back(MenuItem::separator());
            m.items.push_back(makeAction("find-in-file", "Find"));
            m.items.push_back(makeAction("find-in-project", "Find in Project"));
            bar.addMenu(m);
        }

        // View
        {
            Menu m;
            m.title = "View";
            m.items.push_back(makeAction("toggle-file-tree", "Toggle File Tree"));
            m.items.push_back(makeAction("toggle-ast-view", "Toggle AST"));
            m.items.push_back(makeAction("toggle-diagnostics", "Toggle Diagnostics"));
            m.items.push_back(makeAction("toggle-annotations", "Toggle Annotations"));
            m.items.push_back(MenuItem::separator());
            m.items.push_back(makeAction("reset-layout", "Reset Layout"));
            bar.addMenu(m);
        }

        // Tools
        {
            Menu m;
            m.title = "Tools";
            m.items.push_back(makeAction("run-pipeline", "Run Pipeline"));
            m.items.push_back(makeAction("generate-code", "Generate Code"));
            m.items.push_back(makeAction("infer-annotations", "Infer Annotations"));
            m.items.push_back(makeAction("validate", "Validate"));
            m.items.push_back(MenuItem::separator());
            m.items.push_back(makeAction("export-semanno", "Export Semanno"));
            bar.addMenu(m);
        }

        // Workflow
        {
            Menu m;
            m.title = "Workflow";
            m.items.push_back(makeAction("create-workflow", "Create Workflow"));
            m.items.push_back(makeAction("route-all", "Route All"));
            m.items.push_back(makeAction("view-ready-tasks", "View Ready Tasks"));
            m.items.push_back(makeAction("save-workflow", "Save Workflow"));
            bar.addMenu(m);
        }

        // Help with submenu
        {
            Menu m;
            m.title = "Help";
            MenuItem about = makeAction("about", "About");
            MenuItem shortcuts = makeAction("keyboard-shortcuts", "Keyboard Shortcuts");

            MenuItem docs;
            docs.id = "docs";
            docs.label = "Documentation";
            docs.hasSubmenu = true;
            docs.submenuItems.push_back(makeAction("docs-getting-started", "Getting Started"));
            docs.submenuItems.push_back(makeAction("docs-keybindings", "Keybindings"));

            m.items.push_back(about);
            m.items.push_back(shortcuts);
            m.items.push_back(MenuItem::separator());
            m.items.push_back(docs);
            bar.addMenu(m);
        }

        return bar;
    }

private:
    std::vector<Menu> menus_;
    std::string openMenu_;
};
