#pragma once
// Step 680: Score taskitem self-containment quality.

#include "PrerequisiteOpResolver.h"

#include <algorithm>
#include <string>
#include <vector>

struct ScoredTaskitem {
    std::string taskId;
    int score = 0;
    int executionSpecificityScore = 0;
    bool selfContained = false;
    std::vector<std::string> issues;
};

struct TaskitemInput {
    std::string taskId;
    std::string title;
    std::vector<std::string> prerequisiteOps;
    std::vector<std::string> reasons;
    int confidence = 0;
    std::vector<std::string> dependencyTaskIds;
    int executionSpecificityScore = 0;
    bool strictExecutionContract = false;
    bool capabilityCompileFailed = false;
    bool capabilityTestsFailed = false;
    bool capabilityToolchainMissing = false;
};

class SelfContainmentScorer {
public:
    static ScoredTaskitem score(const TaskitemInput& item,
                                const std::string& workspaceRoot) {
        ScoredTaskitem out;
        out.taskId = item.taskId;
        int scoreValue = 100;

        if (item.prerequisiteOps.empty()) {
            scoreValue -= 30;
            out.issues.push_back("prerequisiteOps missing or empty");
        }

        int unresolvedDeduction = 0;
        if (!item.prerequisiteOps.empty()) {
            auto resolved = PrerequisiteOpResolver::resolveAll(item.prerequisiteOps, workspaceRoot);
            for (const auto& op : resolved) {
                if (op.kind == OpKind::ReadFile && !op.fileExists) {
                    unresolvedDeduction += 10;
                }
            }
        }
        unresolvedDeduction = std::min(unresolvedDeduction, 30);
        if (unresolvedDeduction > 0) {
            scoreValue -= unresolvedDeduction;
            out.issues.push_back("prerequisiteOps include unresolved file paths");
        }

        if (item.reasons.empty()) {
            scoreValue -= 20;
            out.issues.push_back("reasons missing or empty");
        } else if (item.reasons.size() == 1) {
            scoreValue -= 25;
            out.issues.push_back("reasons has fewer than 2 entries");
        } else if (item.reasons.size() < 3) {
            scoreValue -= 10;
            out.issues.push_back("reasons has fewer than 3 entries");
        }

        if (item.confidence == 0) {
            scoreValue -= 10;
            out.issues.push_back("confidence not set");
        }

        if (item.title.empty()) {
            scoreValue -= 20;
            out.issues.push_back("title empty");
        }

        if (!item.dependencyTaskIds.empty()) {
            scoreValue -= 5;
            out.issues.push_back("dependencyTaskIds not empty");
        }

        if (item.strictExecutionContract) {
            if (item.executionSpecificityScore < 60) {
                scoreValue -= 25;
                out.issues.push_back("executionSpecificityScore below strict threshold");
            }
            if (item.executionSpecificityScore == 0) {
                scoreValue -= 10;
                out.issues.push_back("execution contract metadata missing");
            }
        }

        if (item.capabilityCompileFailed || item.capabilityTestsFailed || item.capabilityToolchainMissing) {
            scoreValue -= 25;
            out.issues.push_back("capability_signals indicate generator/runtime failure");
        }

        out.score = std::clamp(scoreValue, 0, 100);
        out.executionSpecificityScore = std::clamp(item.executionSpecificityScore, 0, 100);
        out.selfContained = out.score >= 80;
        return out;
    }
};
