#pragma once
// Step 767: Family promotion checks (experimental -> beta).

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct ManagedFamilyPairTier {
    std::string source;
    std::string target;
    std::string tier;
};

struct ManagedFamilyPromotionReport {
    int pairCount = 0;
    int betaCount = 0;
    int experimentalCount = 0;
    nlohmann::json pairs = nlohmann::json::array();
};

class ManagedFamilyPromotionMatrix {
public:
    static std::vector<ManagedFamilyPairTier> defaultPairs() {
        return {
            {"kotlin", "csharp", "beta"},
            {"kotlin", "fsharp", "beta"},
            {"kotlin", "vbnet", "experimental"},
            {"csharp", "kotlin", "beta"},
            {"csharp", "fsharp", "beta"},
            {"csharp", "vbnet", "beta"},
            {"fsharp", "kotlin", "beta"},
            {"fsharp", "csharp", "beta"},
            {"fsharp", "vbnet", "experimental"},
            {"vbnet", "kotlin", "experimental"},
            {"vbnet", "csharp", "beta"},
            {"vbnet", "fsharp", "experimental"}
        };
    }

    static std::vector<ManagedFamilyPairTier> sorted(std::vector<ManagedFamilyPairTier> rows) {
        std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
            if (a.source != b.source) return a.source < b.source;
            return a.target < b.target;
        });
        return rows;
    }

    static std::string lookupTier(const std::string& source, const std::string& target) {
        for (const auto& row : defaultPairs()) {
            if (row.source == source && row.target == target) return row.tier;
        }
        return "experimental";
    }

    static ManagedFamilyPromotionReport evaluate(const std::vector<ManagedFamilyPairTier>& rows) {
        ManagedFamilyPromotionReport r;
        auto normalized = sorted(rows);
        r.pairCount = static_cast<int>(normalized.size());
        for (const auto& p : normalized) {
            if (p.tier == "beta") ++r.betaCount;
            if (p.tier == "experimental") ++r.experimentalCount;
            r.pairs.push_back({{"source", p.source}, {"target", p.target}, {"tier", p.tier}});
        }
        return r;
    }

    static nlohmann::json toJson(const ManagedFamilyPromotionReport& r) {
        return {
            {"pair_count", r.pairCount},
            {"beta_count", r.betaCount},
            {"experimental_count", r.experimentalCount},
            {"pairs", r.pairs}
        };
    }
};
