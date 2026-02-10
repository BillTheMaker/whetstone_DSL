#pragma once
// Step 114: Go-to-line parsing helpers

#include <string>
#include <cctype>
#include "StringUtils.h"

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
