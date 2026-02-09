#pragma once
#include <string>
#include <vector>

struct LineDiff {
    std::vector<int> beforeLines;
    std::vector<int> afterLines;
};

inline std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::string current;
    for (char c : text) {
        if (c == '\n') {
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    lines.push_back(current);
    return lines;
}

inline LineDiff buildLineDiff(const std::string& beforeText, const std::string& afterText) {
    LineDiff diff;
    auto before = splitLines(beforeText);
    auto after = splitLines(afterText);
    size_t maxLines = before.size() > after.size() ? before.size() : after.size();
    for (size_t i = 0; i < maxLines; ++i) {
        const std::string* b = i < before.size() ? &before[i] : nullptr;
        const std::string* a = i < after.size() ? &after[i] : nullptr;
        if (!b || !a || *b != *a) {
            if (b) diff.beforeLines.push_back((int)i);
            if (a) diff.afterLines.push_back((int)i);
        }
    }
    return diff;
}
