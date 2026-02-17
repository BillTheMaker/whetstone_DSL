#pragma once
// Sprint 32 refactor: shared intake text normalization helpers

#include <algorithm>
#include <cctype>
#include <string>

inline std::string intakeToLower(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

inline std::string intakeCompactLowerAlnumSpaces(const std::string& text) {
    std::string out;
    bool lastSpace = false;
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            lastSpace = false;
        } else if (!lastSpace) {
            out.push_back(' ');
            lastSpace = true;
        }
    }
    while (!out.empty() && out.front() == ' ') out.erase(out.begin());
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

inline std::string intakeSanitizeToken(const std::string& text, char separator) {
    std::string out;
    bool lastSep = false;
    for (char c : intakeToLower(text)) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out.push_back(c);
            lastSep = false;
        } else if (!lastSep) {
            out.push_back(separator);
            lastSep = true;
        }
    }
    while (!out.empty() && out.front() == separator) out.erase(out.begin());
    while (!out.empty() && out.back() == separator) out.pop_back();
    return out;
}
