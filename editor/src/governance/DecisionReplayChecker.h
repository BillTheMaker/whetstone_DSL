#pragma once
// Step 824: Decision replay and reproducibility checker.

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct DecisionSnapshot {
    std::string snapshotId;
    std::string issueRef;
    std::string policy;
    std::string reviewer;
    std::string decision;
    std::string rationale;
};

struct ReplayResult {
    std::string snapshotId;
    bool policyMatch = false;
    bool reviewerMatch = false;
    bool decisionMatch = false;
    bool fullyReproducible = false;
    std::string divergenceReason;
};

class DecisionReplayChecker {
public:
    static ReplayResult replay(const DecisionSnapshot& archived,
                               const DecisionSnapshot& current) {
        ReplayResult r;
        r.snapshotId = archived.snapshotId;
        r.policyMatch   = archived.policy   == current.policy;
        r.reviewerMatch = archived.reviewer == current.reviewer;
        r.decisionMatch = archived.decision == current.decision;
        r.fullyReproducible = r.policyMatch && r.reviewerMatch && r.decisionMatch;
        if (!r.policyMatch)   r.divergenceReason = "policy_changed";
        else if (!r.reviewerMatch) r.divergenceReason = "reviewer_changed";
        else if (!r.decisionMatch) r.divergenceReason = "decision_changed";
        return r;
    }

    static bool validate(const DecisionSnapshot& s, std::string* error) {
        if (!error) return false;
        error->clear();
        if (s.snapshotId.empty()) { *error = "snapshot_id_missing"; return false; }
        if (s.issueRef.empty())   { *error = "issue_ref_missing";   return false; }
        if (s.decision.empty())   { *error = "decision_missing";    return false; }
        return true;
    }

    static std::vector<ReplayResult> batchReplay(
        const std::vector<DecisionSnapshot>& archived,
        const std::vector<DecisionSnapshot>& current) {
        std::vector<ReplayResult> out;
        for (size_t i = 0; i < archived.size() && i < current.size(); ++i)
            out.push_back(replay(archived[i], current[i]));
        return out;
    }

    static int countReproducible(const std::vector<ReplayResult>& results) {
        int n = 0;
        for (const auto& r : results) if (r.fullyReproducible) ++n;
        return n;
    }

    static nlohmann::json toJson(const ReplayResult& r) {
        return {{"snapshot_id", r.snapshotId}, {"policy_match", r.policyMatch},
                {"reviewer_match", r.reviewerMatch}, {"decision_match", r.decisionMatch},
                {"fully_reproducible", r.fullyReproducible}, {"divergence_reason", r.divergenceReason}};
    }
};
