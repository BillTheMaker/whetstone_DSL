#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct CommandEntry {
    std::string id;
    std::string label;
    std::string shortcut;
    int lastUsed = 0;
};

class CommandPalette {
public:
    void registerCommand(const std::string& id,
                         const std::string& label,
                         const std::string& shortcut) {
        auto& entry = commands_[id];
        entry.id = id;
        entry.label = label;
        entry.shortcut = shortcut;
    }

    void markUsed(const std::string& id) {
        auto it = commands_.find(id);
        if (it == commands_.end()) return;
        it->second.lastUsed = ++useCounter_;
    }

    std::vector<CommandEntry> search(const std::string& query) const {
        std::vector<CommandEntry> results;
        for (const auto& [_, cmd] : commands_) {
            if (query.empty()) {
                results.push_back(cmd);
            } else {
                int score = fuzzyScore(cmd.label, query);
                if (score > 0) {
                    CommandEntry ranked = cmd;
                    ranked.lastUsed = cmd.lastUsed + score * 1000;
                    results.push_back(ranked);
                }
            }
        }

        std::sort(results.begin(), results.end(), [](const CommandEntry& a, const CommandEntry& b) {
            if (a.lastUsed != b.lastUsed) return a.lastUsed > b.lastUsed;
            return a.label < b.label;
        });
        return results;
    }

    std::vector<CommandEntry> all() const {
        std::vector<CommandEntry> result;
        for (const auto& [_, cmd] : commands_) result.push_back(cmd);
        return result;
    }

private:
    static int fuzzyScore(const std::string& text, const std::string& query) {
        if (query.empty()) return 1;
        int score = 0;
        size_t pos = 0;
        for (char qc : query) {
            char q = toLower(qc);
            bool found = false;
            while (pos < text.size()) {
                char t = toLower(text[pos]);
                if (t == q) {
                    score += 10;
                    found = true;
                    ++pos;
                    break;
                }
                ++pos;
            }
            if (!found) return 0;
        }
        return score;
    }

    static char toLower(char c) {
        if (c >= 'A' && c <= 'Z') return char(c - 'A' + 'a');
        return c;
    }

    std::unordered_map<std::string, CommandEntry> commands_;
    int useCounter_ = 0;
};
