#pragma once
// Step 1490: campaign target prioritizer.

#include <algorithm>
#include <string>
#include <vector>

struct CampaignTarget {
    std::string target;
    int severity = 0;
    int flakiness = 0;
};

class CampaignTargetPrioritizer {
public:
    static std::vector<CampaignTarget> prioritize(std::vector<CampaignTarget> in) {
        std::sort(in.begin(), in.end(), [](const auto& a, const auto& b) {
            if (a.severity != b.severity) return a.severity > b.severity;
            if (a.flakiness != b.flakiness) return a.flakiness < b.flakiness;
            return a.target < b.target;
        });
        return in;
    }
};
