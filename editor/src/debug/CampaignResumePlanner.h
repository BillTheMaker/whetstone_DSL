#pragma once
// Step 1502: campaign resume planner model.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "CampaignCheckpoint.h"

struct CampaignResumePlan {
    bool canResume = false;
    std::string campaignId;
    int resumedFromSequence = 0;
    CampaignProgress restoredProgress;
};

class CampaignResumePlanner {
public:
    static CampaignResumePlan plan(const std::string& campaignId,
                                   const std::vector<CampaignCheckpoint>& checkpoints) {
        CampaignResumePlan p;
        p.campaignId = campaignId;

        CampaignCheckpoint best;
        bool found = false;
        for (const auto& c : checkpoints) {
            if (c.campaignId != campaignId) continue;
            if (!found || c.sequence > best.sequence ||
                (c.sequence == best.sequence && c.checkpointId > best.checkpointId)) {
                best = c;
                found = true;
            }
        }

        if (found) {
            p.canResume = true;
            p.resumedFromSequence = best.sequence;
            p.restoredProgress = best.progress;
            p.restoredProgress.stopped = false;
        }
        return p;
    }

    static nlohmann::json toJson(const CampaignResumePlan& p) {
        return {
            {"can_resume", p.canResume},
            {"campaign_id", p.campaignId},
            {"resumed_from_sequence", p.resumedFromSequence},
            {"restored_progress", CampaignProgressTracker::toJson(p.restoredProgress)}
        };
    }
};
