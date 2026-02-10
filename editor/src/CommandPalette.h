#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct CommandEntry {
    std::string id;
    std::string label;
    std::string shortcut;
    std::string category;
    std::string inputHint;
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
    void registerCommand(const std::string& id,
                         const std::string& label,
                         const std::string& shortcut,
                         const std::string& category,
                         int contextMask,
                         const std::string& inputHint,
                         bool inlineInput) {
        auto& entry = commands_[id];
        entry.id = id;
        entry.label = label;
        entry.shortcut = shortcut;
        entry.category = category;
        entry.contextMask = contextMask;
        entry.inputHint = inputHint;
        entry.inlineInput = inlineInput;
    }

    void markUsed(const std::string& id) {
        auto it = commands_.find(id);
        if (it == commands_.end()) return;
        it->second.lastUsed = ++useCounter_;
    }

    std::vector<CommandMatch> search(const std::string& query,
                                     int contextMask,
                                     bool strictContext) const {
        std::vector<CommandMatch> results;
        for (const auto& [_, cmd] : commands_) {
            const bool contextOk = matchesContext(cmd, contextMask);
            if (strictContext && !contextOk) continue;
            CommandMatch match;
            match.entry = cmd;
            if (query.empty()) {
                match.score = cmd.lastUsed > 0 ? 50 + cmd.lastUsed : 1;
            } else {
                int score = 0;
                if (!fuzzyMatch(cmd.label, query, score, match.matchIndices)) continue;
                match.score = score;
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
};
