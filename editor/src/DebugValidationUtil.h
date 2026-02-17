#pragma once
// Sprint 30 refactor: shared debug validation helpers

#include <cstdint>
#include <string>

inline bool hasRequiredDebugIds(const std::string& a, const std::string& b) {
    return !a.empty() && !b.empty();
}

inline bool hasRequiredDebugIds(const std::string& a,
                                const std::string& b,
                                const std::string& c) {
    return hasRequiredDebugIds(a, b) && !c.empty();
}

inline bool isPositiveLine(int line) {
    return line > 0;
}

inline bool isNonZeroU64(std::uint64_t value) {
    return value > 0;
}
