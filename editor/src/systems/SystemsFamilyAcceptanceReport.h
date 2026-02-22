#pragma once
// Step 747: family-level acceptance report generator.

#include <nlohmann/json.hpp>

#include "SystemsCompatibilityMatrix.h"

struct SystemsFamilyAcceptanceReport {
    int pairCount = 0;
    int stableCount = 0;
    int betaCount = 0;
    nlohmann::json pairs = nlohmann::json::array();
};

class SystemsFamilyAcceptanceReportModel {
public:
    static SystemsFamilyAcceptanceReport build(const std::vector<SystemPairCompatibility>& rows) {
        SystemsFamilyAcceptanceReport r;
        auto sorted = SystemsCompatibilityMatrix::sorted(rows);
        r.pairCount = static_cast<int>(sorted.size());
        for (const auto& p : sorted) {
            if (p.tier == "stable") ++r.stableCount;
            if (p.tier == "beta") ++r.betaCount;
        }
        r.pairs = SystemsCompatibilityMatrix::toJson(sorted);
        return r;
    }

    static nlohmann::json toJson(const SystemsFamilyAcceptanceReport& r) {
        return {{"pair_count", r.pairCount}, {"stable_count", r.stableCount}, {"beta_count", r.betaCount}, {"pairs", r.pairs}};
    }
};
