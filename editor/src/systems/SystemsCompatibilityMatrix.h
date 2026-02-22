#pragma once
// Step 745: cross-pair compatibility matrix update.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct SystemPairCompatibility {
    std::string source;
    std::string target;
    std::string tier;
};

class SystemsCompatibilityMatrix {
public:
    static std::vector<SystemPairCompatibility> defaultPairs() {
        return {
            {"c", "rust", "beta"},
            {"c", "cpp", "stable"},
            {"c", "go", "beta"},
            {"go", "rust", "beta"},
            {"go", "cpp", "stable"},
            {"java", "cpp", "beta"},
            {"java", "rust", "beta"},
            {"java", "go", "beta"}
        };
    }

    static std::vector<SystemPairCompatibility> sorted(std::vector<SystemPairCompatibility> rows) {
        std::sort(rows.begin(), rows.end(), [](const SystemPairCompatibility& a, const SystemPairCompatibility& b) {
            if (a.source != b.source) return a.source < b.source;
            return a.target < b.target;
        });
        return rows;
    }

    static nlohmann::json toJson(const std::vector<SystemPairCompatibility>& rows) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& r : sorted(rows)) arr.push_back({{"source", r.source}, {"target", r.target}, {"tier", r.tier}});
        return arr;
    }
};
