#pragma once
// Step 1497: campaign report bundle.

#include <string>

#include <nlohmann/json.hpp>

#include "CampaignProgressTracker.h"

struct CampaignReportBundle {
    std::string campaignId;
    CampaignProgress progress;
    nlohmann::json metrics = nlohmann::json::object();
};

class CampaignReportBundleModel {
public:
    static CampaignReportBundle build(const std::string& campaignId,
                                      const CampaignProgress& progress,
                                      const nlohmann::json& metrics) {
        CampaignReportBundle b;
        b.campaignId = campaignId;
        b.progress = progress;
        b.metrics = metrics;
        return b;
    }

    static nlohmann::json toJson(const CampaignReportBundle& b) {
        return {{"campaign_id", b.campaignId}, {"progress", CampaignProgressTracker::toJson(b.progress)}, {"metrics", b.metrics}};
    }
};
