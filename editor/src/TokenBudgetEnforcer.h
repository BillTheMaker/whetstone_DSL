#pragma once
// Step 676: Token budget enforcement for assembled context slices.

#include "ContextSliceAssembler.h"

#include <string>
#include <utility>
#include <vector>

struct BudgetReport {
    int totalSlices = 0;
    int includedSlices = 0;
    int droppedSlices = 0;
    int estimatedTokensUsed = 0;
    int budgetTokens = 0;
    bool budgetExceeded = false;
    std::vector<std::string> droppedFiles;
};

class TokenBudgetEnforcer {
public:
    static std::pair<std::vector<ContextSlice>, BudgetReport>
    enforce(const std::vector<ContextSlice>& slices, int maxTokens) {
        std::vector<ContextSlice> included;
        BudgetReport report;
        report.totalSlices = static_cast<int>(slices.size());
        report.budgetTokens = maxTokens < 0 ? 0 : maxTokens;

        int used = 0;
        for (const auto& slice : slices) {
            const int tokens = estimateTokens(slice.content);
            if (used + tokens <= report.budgetTokens) {
                included.push_back(slice);
                used += tokens;
                report.includedSlices++;
            } else {
                report.droppedSlices++;
                report.budgetExceeded = true;
                report.droppedFiles.push_back(slice.filePath);
            }
        }

        report.estimatedTokensUsed = used;
        return {included, report};
    }

    static int estimateTokens(const std::string& text) {
        if (text.empty()) return 0;
        return static_cast<int>((text.size() + 3U) / 4U);
    }
};

