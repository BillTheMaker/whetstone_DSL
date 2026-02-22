#pragma once
// Step 1481: bisect candidate selector.

#include <algorithm>
#include <string>
#include <vector>

struct BisectCandidate {
    std::string proposalId;
    int index = 0;
};

class BisectCandidateSelector {
public:
    static std::vector<BisectCandidate> ordered(const std::vector<std::string>& proposalIds) {
        std::vector<BisectCandidate> out;
        for (size_t i = 0; i < proposalIds.size(); ++i) out.push_back({proposalIds[i], static_cast<int>(i)});
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.proposalId < b.proposalId; });
        return out;
    }

    static BisectCandidate midpoint(const std::vector<BisectCandidate>& orderedCandidates) {
        if (orderedCandidates.empty()) return {"", -1};
        return orderedCandidates[orderedCandidates.size() / 2];
    }
};
