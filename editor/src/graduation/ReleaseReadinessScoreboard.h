#pragma once
// Step 832: Release readiness scoreboard.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct ReadinessScore {
    std::string pairId;
    int testCoverage = 0;  // 0-100
    int docCompleteness = 0;
    int gatePassRate = 0;
    int overallScore = 0;
    std::string tier;
    bool releaseReady = false;
};

class ReleaseReadinessScoreboard {
public:
    static ReadinessScore compute(const std::string& pairId,
                                  int testCoverage, int docCompleteness, int gatePassRate) {
        ReadinessScore s;
        s.pairId = pairId;
        s.testCoverage = testCoverage;
        s.docCompleteness = docCompleteness;
        s.gatePassRate = gatePassRate;
        s.overallScore = (testCoverage + docCompleteness + gatePassRate) / 3;
        s.releaseReady = s.overallScore >= 80;
        s.tier = s.overallScore >= 90 ? "stable" : (s.overallScore >= 70 ? "beta" : "experimental");
        return s;
    }

    static std::vector<ReadinessScore> rank(std::vector<ReadinessScore> scores) {
        std::sort(scores.begin(), scores.end(),
            [](const ReadinessScore& a, const ReadinessScore& b) {
                return a.overallScore > b.overallScore;
            });
        return scores;
    }

    static nlohmann::json toJson(const ReadinessScore& s) {
        return {{"pair_id", s.pairId}, {"test_coverage", s.testCoverage},
                {"doc_completeness", s.docCompleteness}, {"gate_pass_rate", s.gatePassRate},
                {"overall_score", s.overallScore}, {"tier", s.tier},
                {"release_ready", s.releaseReady}};
    }
};
