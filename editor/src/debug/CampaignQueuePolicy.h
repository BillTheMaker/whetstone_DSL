#pragma once
// Step 1501: campaign queue policy model.

#include <algorithm>
#include <string>
#include <vector>

struct CampaignQueueItem {
    std::string campaignId;
    int priority = 0;
    int ageMinutes = 0;
};

class CampaignQueuePolicy {
public:
    static std::vector<CampaignQueueItem> order(std::vector<CampaignQueueItem> items) {
        std::sort(items.begin(), items.end(), [](const CampaignQueueItem& a, const CampaignQueueItem& b) {
            if (a.priority != b.priority) return a.priority > b.priority;
            if (a.ageMinutes != b.ageMinutes) return a.ageMinutes > b.ageMinutes;
            return a.campaignId < b.campaignId;
        });
        return items;
    }
};
