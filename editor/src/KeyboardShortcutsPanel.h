#pragma once
// Step 355: Keyboard shortcuts panel model.
// Headless grouping/filtering/customize behavior for shortcut display.

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "KeybindingRegistry.h"

struct ShortcutRow {
    std::string actionId;
    std::string label;
    std::string category;
    std::string symbols;
    std::string description;
};

class KeyboardShortcutsPanel {
public:
    void open() { visible_ = true; }
    void close() { visible_ = false; }
    bool isVisible() const { return visible_; }

    void setFilter(const std::string& text) { filter_ = text; }
    const std::string& filter() const { return filter_; }

    void loadFromRegistry(const KeybindingRegistry& registry, bool macOS = false) {
        rows_.clear();
        for (const auto& action : registry.getActions()) {
            ShortcutRow row;
            row.actionId = action.id;
            row.label = action.label;
            row.category = action.category;
            row.description = action.description;
            row.symbols = registry.getSymbols(action.id, macOS);
            rows_.push_back(row);
        }
        std::sort(rows_.begin(), rows_.end(), [](const ShortcutRow& a, const ShortcutRow& b) {
            if (a.category != b.category) return a.category < b.category;
            return a.label < b.label;
        });
    }

    const std::vector<ShortcutRow>& rows() const { return rows_; }

    std::vector<ShortcutRow> filteredRows() const {
        if (filter_.empty()) return rows_;
        std::string needle = toLower(filter_);
        std::vector<ShortcutRow> out;
        for (const auto& row : rows_) {
            std::string hay = toLower(row.label + " " + row.category + " " + row.actionId);
            if (hay.find(needle) != std::string::npos) out.push_back(row);
        }
        return out;
    }

    std::map<std::string, std::vector<ShortcutRow>> groupedRows() const {
        std::map<std::string, std::vector<ShortcutRow>> groups;
        for (const auto& row : filteredRows()) {
            groups[row.category].push_back(row);
        }
        return groups;
    }

    void requestCustomize() { customizeRequested_ = true; }
    bool consumeCustomizeRequest() {
        bool value = customizeRequested_;
        customizeRequested_ = false;
        return value;
    }

private:
    static std::string toLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    bool visible_ = false;
    bool customizeRequested_ = false;
    std::string filter_;
    std::vector<ShortcutRow> rows_;
};
