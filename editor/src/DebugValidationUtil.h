#pragma once
// Sprint 30 refactor: shared debug validation helpers

#include <string>

inline bool hasRequiredDebugIds(const std::string& a, const std::string& b) {
    return !a.empty() && !b.empty();
}

inline bool isPositiveLine(int line) {
    return line > 0;
}
