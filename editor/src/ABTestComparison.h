#pragma once
// Step 686: A/B comparison between baseline and whetstone-assisted runs.

#include "AgentSessionRecorder.h"

#include <string>

struct ABComparisonResult {
    std::string taskId;
    int baselineTokens = 0;
    int whetstoneTokens = 0;
    double tokenReductionPct = 0.0;
    int baselineFileReads = 0;
    int whetstoneFileReads = 0;
    double fileReadReductionPct = 0.0;
    long long baselineDurationMs = 0;
    long long whetstoneDurationMs = 0;
    double durationDeltaPct = 0.0;
    int baselineQuality = 0;
    int whetstoneQuality = 0;
    int qualityDelta = 0;
    bool whetstoneWon = false;
};

class ABTestComparison {
public:
    static ABComparisonResult compare(const std::string& taskId,
                                      const SessionRecord& baseline,
                                      const SessionRecord& whetstone,
                                      int baselineQuality,
                                      int whetstoneQuality) {
        ABComparisonResult out;
        out.taskId = taskId;

        out.baselineTokens = baseline.totalEstimatedTokens;
        out.whetstoneTokens = whetstone.totalEstimatedTokens;
        out.tokenReductionPct = pctReduction(baseline.totalEstimatedTokens, whetstone.totalEstimatedTokens);

        out.baselineFileReads = baseline.fileReadCount;
        out.whetstoneFileReads = whetstone.fileReadCount;
        out.fileReadReductionPct = pctReduction(baseline.fileReadCount, whetstone.fileReadCount);

        out.baselineDurationMs = baseline.totalDurationMs;
        out.whetstoneDurationMs = whetstone.totalDurationMs;
        out.durationDeltaPct = pctReduction(static_cast<double>(baseline.totalDurationMs),
                                            static_cast<double>(whetstone.totalDurationMs));

        out.baselineQuality = baselineQuality;
        out.whetstoneQuality = whetstoneQuality;
        out.qualityDelta = whetstoneQuality - baselineQuality;

        out.whetstoneWon = out.tokenReductionPct > 0.0 && out.qualityDelta >= 0;
        return out;
    }

private:
    static double pctReduction(double baseline, double compareValue) {
        if (baseline <= 0.0) return 0.0;
        return ((baseline - compareValue) / baseline) * 100.0;
    }
};

