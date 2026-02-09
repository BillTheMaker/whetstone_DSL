#pragma once
// Step 114: Go-to-line parsing helpers

#include <string>
#include <cctype>

inline std::string trimCopy(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start])) ++start;
    size_t end = s.size();
    while (end > start && std::isspace((unsigned char)s[end - 1])) --end;
    return s.substr(start, end - start);
}

inline bool parseLineColInput(const std::string& input, int& outLine, int& outCol) {
    std::string s = trimCopy(input);
    if (s.empty()) return false;
    if (!s.empty() && s[0] == ':') s = s.substr(1);

    size_t colon = s.find(':');
    std::string lineStr = (colon == std::string::npos) ? s : s.substr(0, colon);
    std::string colStr = (colon == std::string::npos) ? "" : s.substr(colon + 1);

    if (lineStr.empty()) return false;
    int line = 0;
    for (char c : lineStr) {
        if (!std::isdigit((unsigned char)c)) return false;
        line = line * 10 + (c - '0');
    }
    if (line <= 0) return false;

    int col = 1;
    if (!colStr.empty()) {
        col = 0;
        for (char c : colStr) {
            if (!std::isdigit((unsigned char)c)) return false;
            col = col * 10 + (c - '0');
        }
        if (col <= 0) return false;
    }

    outLine = line;
    outCol = col;
    return true;
}
