#pragma once
// Step 757: dynamic family acceptance report + review queue wiring.

#include <nlohmann/json.hpp>

#include "PythonAdapterV2.h"
#include "DynamicRiskClassifier.h"

struct DynamicFamilyAcceptanceReport {
    int totalItems = 0;
    int reviewRequiredCount = 0;
    nlohmann::json reviewQueue = nlohmann::json::array();
};

class DynamicFamilyAcceptanceReportModel {
public:
    static DynamicFamilyAcceptanceReport build(const std::vector<DynamicLoweringPacket>& packets,
                                               const std::vector<DynamicRiskPacket>& risks) {
        DynamicFamilyAcceptanceReport r;
        r.totalItems = static_cast<int>(packets.size());
        for (size_t i = 0; i < packets.size(); ++i) {
            bool review = packets[i].reviewRequired || (i < risks.size() && risks[i].level != "low");
            if (review) {
                ++r.reviewRequiredCount;
                r.reviewQueue.push_back({{"source_language", packets[i].sourceLanguage},
                                         {"ir_summary", packets[i].irSummary},
                                         {"risk_level", i < risks.size() ? risks[i].level : "unknown"}});
            }
        }
        return r;
    }

    static nlohmann::json toJson(const DynamicFamilyAcceptanceReport& r) {
        return {{"total_items", r.totalItems}, {"review_required_count", r.reviewRequiredCount}, {"review_queue", r.reviewQueue}};
    }
};
