#pragma once
// Step 1499: campaign checkpoint model.

#include <algorithm>
#include <string>

#include <nlohmann/json.hpp>

#include "CampaignProgressTracker.h"

struct CampaignCheckpoint {
    std::string checkpointId;
    std::string campaignId;
    int sequence = 0;
    CampaignProgress progress;
    std::string notes;
};

class CampaignCheckpointModel {
public:
    static CampaignCheckpoint make(const std::string& campaignId,
                                   int sequence,
                                   const CampaignProgress& progress,
                                   const std::string& notes) {
        CampaignCheckpoint c;
        c.campaignId = campaignId;
        c.sequence = sequence;
        c.progress = progress;
        c.notes = notes;
        c.checkpointId = stableId(campaignId, sequence, notes);
        return c;
    }

    static nlohmann::json toJson(const CampaignCheckpoint& c) {
        return {
            {"checkpoint_id", c.checkpointId},
            {"campaign_id", c.campaignId},
            {"sequence", c.sequence},
            {"progress", CampaignProgressTracker::toJson(c.progress)},
            {"notes", c.notes}
        };
    }

    static CampaignCheckpoint fromJson(const nlohmann::json& j) {
        CampaignCheckpoint c;
        c.checkpointId = j.value("checkpoint_id", "");
        c.campaignId = j.value("campaign_id", "");
        c.sequence = j.value("sequence", 0);
        c.notes = j.value("notes", "");
        auto p = j.value("progress", nlohmann::json::object());
        c.progress.campaignId = p.value("campaign_id", c.campaignId);
        c.progress.totalTargets = p.value("total_targets", 0);
        c.progress.completedTargets = p.value("completed_targets", 0);
        c.progress.greenTargets = p.value("green_targets", 0);
        c.progress.escalatedTargets = p.value("escalated_targets", 0);
        c.progress.stopped = p.value("stopped", false);
        return c;
    }

private:
    static std::string stableId(const std::string& campaignId, int sequence, const std::string& notes) {
        uint64_t h = 1469598103934665603ull;
        std::string key = campaignId + "|" + std::to_string(sequence) + "|" + notes;
        for (unsigned char ch : key) {
            h ^= ch;
            h *= 1099511628211ull;
        }
        return "cp_" + std::to_string(h);
    }
};
