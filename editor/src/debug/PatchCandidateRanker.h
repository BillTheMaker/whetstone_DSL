#pragma once
// Step 1470: patch candidate ranking with history penalties.

#include <algorithm>
#include <string>
#include <vector>

struct RankedPatchCandidate {
    std::string proposalId;
    double confidence = 0.0;
    int attempts = 0;
    double score = 0.0;
};

class PatchCandidateRanker {
public:
    static std::vector<RankedPatchCandidate> rank(std::vector<RankedPatchCandidate> in) {
        for (auto& c : in) c.score = c.confidence - 0.1 * c.attempts;
        std::sort(in.begin(), in.end(), [](const auto& a, const auto& b) {
            if (a.score != b.score) return a.score > b.score;
            return a.proposalId < b.proposalId;
        });
        return in;
    }
};
