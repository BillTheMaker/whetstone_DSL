#pragma once
// Step 1492: campaign progress tracker.

#include <string>

#include <nlohmann/json.hpp>

struct CampaignProgress {
    std::string campaignId;
    int totalTargets = 0;
    int completedTargets = 0;
    int greenTargets = 0;
    int escalatedTargets = 0;
    bool stopped = false;
};

class CampaignProgressTracker {
public:
    static double percent(const CampaignProgress& p) {
        if (p.totalTargets <= 0) return 0.0;
        return 100.0 * p.completedTargets / p.totalTargets;
    }

    static nlohmann::json toJson(const CampaignProgress& p) {
        return {{"campaign_id", p.campaignId}, {"total_targets", p.totalTargets}, {"completed_targets", p.completedTargets},
                {"green_targets", p.greenTargets}, {"escalated_targets", p.escalatedTargets},
                {"stopped", p.stopped}, {"percent", percent(p)}};
    }
};
