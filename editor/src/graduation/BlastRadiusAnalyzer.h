#pragma once
// Step 843: Blast-radius analyzer by feature family.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct BlastRadiusEntry {
    std::string featureFamily;
    std::vector<std::string> affectedPairs;
    int impactScore = 0;
};

struct BlastRadiusReport {
    std::string triggerFeature;
    std::vector<BlastRadiusEntry> entries;
    int totalAffectedPairs = 0;
    std::string riskLevel;
};

class BlastRadiusAnalyzer {
public:
    static BlastRadiusReport analyze(const std::string& triggerFeature,
                                     const std::vector<BlastRadiusEntry>& entries) {
        BlastRadiusReport r;
        r.triggerFeature = triggerFeature;
        r.entries = entries;
        for (const auto& e : entries)
            r.totalAffectedPairs += static_cast<int>(e.affectedPairs.size());
        if (r.totalAffectedPairs == 0) r.riskLevel = "none";
        else if (r.totalAffectedPairs < 3) r.riskLevel = "low";
        else if (r.totalAffectedPairs < 8) r.riskLevel = "medium";
        else r.riskLevel = "high";
        return r;
    }

    static nlohmann::json toJson(const BlastRadiusReport& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& e : r.entries) {
            nlohmann::json pairs = nlohmann::json::array();
            for (const auto& p : e.affectedPairs) pairs.push_back(p);
            arr.push_back({{"family", e.featureFamily}, {"pairs", pairs},
                           {"impact", e.impactScore}});
        }
        return {{"trigger", r.triggerFeature}, {"total_affected", r.totalAffectedPairs},
                {"risk_level", r.riskLevel}, {"entries", arr}};
    }
};
