#pragma once
// Step 874: Cost-vs-quality tradeoff report generator.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct TradeoffEntry {
    std::string profile;
    float cost = 0.0f;
    float quality = 0.0f;
    float efficiency = 0.0f; // quality / cost
};

struct CostQualityReport {
    std::string pairId;
    std::vector<TradeoffEntry> entries;
    std::string recommendedProfile;
};

class CostQualityTradeoffReport {
public:
    static CostQualityReport generate(const std::string& pairId,
                                       const std::vector<std::pair<std::string,float>>& planCosts,
                                       const std::vector<std::pair<std::string,float>>& planQualities) {
        CostQualityReport r;
        r.pairId = pairId;
        float bestEff = -1.0f;
        for (size_t i = 0; i < planCosts.size() && i < planQualities.size(); ++i) {
            TradeoffEntry e;
            e.profile = planCosts[i].first;
            e.cost = planCosts[i].second;
            e.quality = planQualities[i].second;
            e.efficiency = (e.cost > 0.0f) ? e.quality / e.cost : 0.0f;
            r.entries.push_back(e);
            if (e.efficiency > bestEff) { bestEff = e.efficiency; r.recommendedProfile = e.profile; }
        }
        return r;
    }

    static nlohmann::json toJson(const CostQualityReport& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& e : r.entries)
            arr.push_back({{"profile", e.profile}, {"cost", e.cost},
                           {"quality", e.quality}, {"efficiency", e.efficiency}});
        return {{"pair_id", r.pairId}, {"entries", arr},
                {"recommended_profile", r.recommendedProfile}};
    }
};
