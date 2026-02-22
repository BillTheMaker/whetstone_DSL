#pragma once
// Step 1506: campaign export bundle model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "CampaignCheckpoint.h"
#include "DebugCampaignSpec.h"

struct CampaignExportBundle {
    DebugCampaignSpec spec;
    CampaignProgress progress;
    std::vector<CampaignCheckpoint> checkpoints;
    nlohmann::json metrics = nlohmann::json::object();
};

class CampaignExportBundleModel {
public:
    static CampaignExportBundle build(const DebugCampaignSpec& spec,
                                      const CampaignProgress& progress,
                                      const std::vector<CampaignCheckpoint>& checkpoints,
                                      const nlohmann::json& metrics) {
        CampaignExportBundle b;
        b.spec = spec;
        b.progress = progress;
        b.checkpoints = checkpoints;
        b.metrics = metrics;
        return b;
    }

    static nlohmann::json toJson(const CampaignExportBundle& b) {
        nlohmann::json cps = nlohmann::json::array();
        for (const auto& c : b.checkpoints) cps.push_back(CampaignCheckpointModel::toJson(c));
        return {
            {"campaign", DebugCampaignSpecModel::toJson(b.spec)},
            {"progress", CampaignProgressTracker::toJson(b.progress)},
            {"checkpoints", cps},
            {"metrics", b.metrics}
        };
    }
};
