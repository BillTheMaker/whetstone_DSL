#pragma once
// Sprint 33 refactor: shared productization validation helpers

#include <string>

inline bool hasRequiredProductIds(const std::string& a, const std::string& b) {
    return !a.empty() && !b.empty();
}

inline bool isValidHexColor7(const std::string& c) {
    if (c.size() != 7 || c[0] != '#') return false;
    for (size_t i = 1; i < c.size(); ++i) {
        const char x = c[i];
        const bool hex = (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F');
        if (!hex) return false;
    }
    return true;
}
