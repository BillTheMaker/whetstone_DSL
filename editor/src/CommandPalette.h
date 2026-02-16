#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include "KeybindingRegistry.h"
#include "PanelManager.h"
#include "Icons.h"

struct CommandEntry {
    std::string id;
    std::string label;
    std::string shortcut;
    std::string symbols;
    std::string icon;
    std::string category;
    std::string inputHint;
    std::vector<std::string> aliases;
    int contextMask = 0;
    bool inlineInput = false;
    int lastUsed = 0;
};

struct CommandMatch {
    CommandEntry entry;
    int score = 0;
    std::vector<int> matchIndices;
};

enum CommandContext : int {
    CommandContext_Any = 0,
    CommandContext_Editor = 1 << 0,
    CommandContext_Search = 1 << 1,
    CommandContext_Settings = 1 << 2,
    CommandContext_Help = 1 << 3,
    CommandContext_Terminal = 1 << 4
};

class CommandPalette {
public:
    void open() {
        isOpen_ = true;
        selectedIndex_ = 0;
    }

    void close() {
        isOpen_ = false;
    }

    bool isOpen() const { return isOpen_; }

    void setQuery(const std::string& q) {
        query_ = q;
        selectedIndex_ = 0;
    }

    const std::string& query() const { return query_; }

    void registerCommand(const std::string& id,
                         const std::string& label,
                         const std::string& shortcut,
                         const std::string& category = "",
                         int contextMask = CommandContext_Any,
                         const std::string& inputHint = "",
                         bool inlineInput = false,
                         const std::vector<std::string>& aliases = {},
                         const std::string& icon = "",
                         const std::string& symbols = "") {
        auto& entry = commands_[id];
        entry.id = id;
        entry.label = label;
        entry.shortcut = shortcut;
        entry.category = category;
        entry.contextMask = contextMask;
        entry.inputHint = inputHint;
        entry.inlineInput = inlineInput;
        entry.aliases = aliases;
        entry.icon = icon;
        entry.symbols = symbols;
    }

    void registerFromKeybindings(const KeybindingRegistry& registry) {
        for (const auto& action : registry.getActions()) {
            const auto binding = registry.getBinding(action.id);
            registerCommand(
                action.id,
                action.label,
                binding.toString(),
                action.category,
                CommandContext_Any,
                "",
                false,
                {action.id},
                WhetstoneIcons::Search,
                binding.toSymbolsAuto()
            );
        }
    }

    void registerPanelToggles(const PanelManager& panels) {
        for (const auto& panel : panels.getPanelList()) {
            std::string id = "toggle-" + panel.id;
            std::string label = "Toggle " + panel.title;
            registerCommand(
                id,
                label,
                "",
                "View",
                CommandContext_Any,
                "",
                false,
                {panel.id, panel.title},
                panel.icon.empty() ? WhetstoneIcons::Search : panel.icon
            );
        }
    }

    void markUsed(const std::string& id) {
        auto it = commands_.find(id);
        if (it == commands_.end()) return;
        it->second.lastUsed = ++useCounter_;
    }

    std::vector<CommandMatch> search(const std::string& query,
                                     int contextMask = CommandContext_Any,
                                     bool strictContext = false) const {
        std::vector<CommandMatch> results;
        for (const auto& [_, cmd] : commands_) {
            const bool contextOk = matchesContext(cmd, contextMask);
            if (strictContext && !contextOk) continue;
            CommandMatch match;
            match.entry = cmd;
            if (query.empty()) {
                match.score = cmd.lastUsed > 0 ? 50 + cmd.lastUsed : 1;
            } else {
                int bestScore = 0;
                std::vector<int> bestIndices;
                bool found = false;

                int score = 0;
                std::vector<int> indices;
                if (fuzzyMatch(cmd.label, query, score, indices)) {
                    found = true;
                    bestScore = score + 20; // label match bias
                    bestIndices = indices;
                }
                if (fuzzyMatch(cmd.id, query, score, indices)) {
                    found = true;
                    if (score + 10 > bestScore) {
                        bestScore = score + 10;
                        bestIndices = indices;
                    }
                }
                if (fuzzyMatch(cmd.category, query, score, indices)) {
                    found = true;
                    if (score + 5 > bestScore) {
                        bestScore = score + 5;
                        bestIndices = indices;
                    }
                }
                for (const auto& alias : cmd.aliases) {
                    if (fuzzyMatch(alias, query, score, indices)) {
                        found = true;
                        if (score + 15 > bestScore) {
                            bestScore = score + 15;
                            bestIndices = indices;
                        }
                    }
                }

                if (!found) continue;
                match.score = bestScore;
                match.matchIndices = bestIndices;
                if (contextOk) match.score += 25;
                else match.score -= 10;
            }
            results.push_back(match);
        }

        std::sort(results.begin(), results.end(), [](const CommandMatch& a, const CommandMatch& b) {
            if (a.score != b.score) return a.score > b.score;
            if (a.entry.lastUsed != b.entry.lastUsed) return a.entry.lastUsed > b.entry.lastUsed;
            return a.entry.label < b.entry.label;
        });
        return results;
    }

    std::vector<CommandEntry> recent(int maxCount, int contextMask) const {
        std::vector<CommandEntry> entries;
        for (const auto& [_, cmd] : commands_) {
            if (cmd.lastUsed <= 0) continue;
            if (!matchesContext(cmd, contextMask)) continue;
            entries.push_back(cmd);
        }
        std::sort(entries.begin(), entries.end(), [](const CommandEntry& a, const CommandEntry& b) {
            if (a.lastUsed != b.lastUsed) return a.lastUsed > b.lastUsed;
            return a.label < b.label;
        });
        if ((int)entries.size() > maxCount) entries.resize(maxCount);
        return entries;
    }

    std::vector<CommandEntry> all() const {
        std::vector<CommandEntry> result;
        for (const auto& [_, cmd] : commands_) result.push_back(cmd);
        return result;
    }

    void moveSelection(int delta, int resultCount) {
        if (resultCount <= 0) {
            selectedIndex_ = 0;
            return;
        }
        selectedIndex_ += delta;
        if (selectedIndex_ < 0) selectedIndex_ = resultCount - 1;
        if (selectedIndex_ >= resultCount) selectedIndex_ = 0;
    }

    int selectedIndex() const { return selectedIndex_; }

    std::optional<std::string> executeSelected(const std::vector<CommandMatch>& matches) {
        if (matches.empty() || selectedIndex_ < 0 || selectedIndex_ >= (int)matches.size()) {
            return std::nullopt;
        }
        const std::string id = matches[selectedIndex_].entry.id;
        markUsed(id);
        return id;
    }

    static bool fuzzyMatch(const std::string& text,
                           const std::string& query,
                           int& outScore,
                           std::vector<int>& outIndices) {
        outScore = 0;
        outIndices.clear();
        if (query.empty()) {
            outScore = 1;
            return true;
        }
        size_t pos = 0;
        int score = 0;
        for (char qc : query) {
            char q = toLower(qc);
            bool found = false;
            while (pos < text.size()) {
                char t = toLower(text[pos]);
                if (t == q) {
                    score += 10;
                    outIndices.push_back((int)pos);
                    found = true;
                    ++pos;
                    break;
                }
                ++pos;
            }
            if (!found) return false;
        }
        outScore = score;
        return score > 0;
    }

private:
    static char toLower(char c) {
        if (c >= 'A' && c <= 'Z') return char(c - 'A' + 'a');
        return c;
    }

    static bool matchesContext(const CommandEntry& entry, int contextMask) {
        if (entry.contextMask == CommandContext_Any) return true;
        return (entry.contextMask & contextMask) != 0;
    }

    std::unordered_map<std::string, CommandEntry> commands_;
    int useCounter_ = 0;
    bool isOpen_ = false;
    std::string query_;
    int selectedIndex_ = 0;
};
