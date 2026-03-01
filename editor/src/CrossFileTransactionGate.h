#pragma once
// Step 1880: Cross-file transaction gate.
// Closes GR-009: long-range edits touch multiple files but DeploymentPromotionGate
// tracked single environments independently. CrossFileTransactionGate makes
// multi-file atomicity explicit: the transaction only promotes if ALL member
// files pass their individual promotion check.

#include "DeploymentPromotionGate.h"

#include <string>
#include <vector>

struct FileEditState {
    std::string fileId;
    int requiredChecks = 0;
    int passedChecks = 0;
    int manualApprovals = 0;
    int blockingIssues = 0;
};

struct TransactionGateResult {
    bool promotable = false;
    std::vector<std::string> blockedFiles;
    int totalFiles = 0;
    int readyFiles = 0;
};

class CrossFileTransactionGate {
public:
    static TransactionGateResult evaluate(const std::vector<FileEditState>& files) {
        TransactionGateResult result;
        result.totalFiles = static_cast<int>(files.size());

        if (files.empty()) {
            result.promotable = true;
            return result;
        }

        DeploymentPromotionGate gate;
        std::string error;

        for (const auto& f : files) {
            EnvironmentGateState state;
            state.environmentId   = f.fileId;
            state.requiredChecks  = f.requiredChecks;
            state.passedChecks    = f.passedChecks;
            state.manualApprovals = f.manualApprovals;
            state.blockingIssues  = f.blockingIssues;
            gate.upsert(state, &error);
        }

        for (const auto& f : files) {
            if (gate.promotable(f.fileId, &error)) {
                ++result.readyFiles;
            } else {
                result.blockedFiles.push_back(f.fileId);
            }
        }

        result.promotable = result.blockedFiles.empty();
        return result;
    }
};
