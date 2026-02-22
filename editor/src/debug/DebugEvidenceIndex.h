#pragma once
// Step 1542: debug evidence index model.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugEvidenceEntry {
    std::string kind;
    std::string ref;
};

class DebugEvidenceIndexModel {
public:
    static std::vector<DebugEvidenceEntry> sort(std::vector<DebugEvidenceEntry> entries) {
        std::sort(entries.begin(), entries.end(), [](const DebugEvidenceEntry& a, const DebugEvidenceEntry& b) {
            if (a.kind != b.kind) return a.kind < b.kind;
            return a.ref < b.ref;
        });
        return entries;
    }

    static nlohmann::json toJson(const std::vector<DebugEvidenceEntry>& entries) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& e : entries) arr.push_back({{"kind", e.kind}, {"ref", e.ref}});
        return arr;
    }
};
