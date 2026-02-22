#pragma once
// Step 1452: patch proposal packet model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct PatchPlanItem {
    std::string intent;
    std::string targetFile;
};

struct PatchProposal {
    std::string proposalId;
    std::string failureClusterId;
    std::vector<PatchPlanItem> patchPlan;
    std::string candidateDiff;
    std::string riskLevel = "medium";
    std::vector<std::string> expectedTestsToRun;
    std::vector<std::string> invariants;
};

class PatchProposalModel {
public:
    static bool validate(const PatchProposal& p,
                         const std::vector<std::string>& allowedFiles,
                         std::string* error) {
        if (error) *error = "";
        if (p.invariants.empty()) {
            if (error) *error = "invariants_missing";
            return false;
        }
        if (p.candidateDiff.empty() || p.candidateDiff.find("diff --git") == std::string::npos) {
            if (error) *error = "diff_invalid";
            return false;
        }
        for (const auto& item : p.patchPlan) {
            bool allowed = false;
            for (const auto& f : allowedFiles) if (item.targetFile == f) allowed = true;
            if (!allowed) {
                if (error) *error = "out_of_scope_edit";
                return false;
            }
        }
        return true;
    }

    static nlohmann::json toJson(const PatchProposal& p) {
        nlohmann::json plan = nlohmann::json::array();
        for (const auto& i : p.patchPlan) plan.push_back({{"intent", i.intent}, {"target_file", i.targetFile}});
        return {
            {"proposal_id", p.proposalId},
            {"failure_cluster_id", p.failureClusterId},
            {"patch_plan", plan},
            {"candidate_diff", p.candidateDiff},
            {"risk_level", p.riskLevel},
            {"expected_tests_to_run", p.expectedTestsToRun},
            {"invariants", p.invariants}
        };
    }

    static PatchProposal fromJson(const nlohmann::json& j) {
        PatchProposal p;
        p.proposalId = j.value("proposal_id", "");
        p.failureClusterId = j.value("failure_cluster_id", "");
        p.candidateDiff = j.value("candidate_diff", "");
        p.riskLevel = j.value("risk_level", "medium");
        p.expectedTestsToRun = j.value("expected_tests_to_run", std::vector<std::string>{});
        p.invariants = j.value("invariants", std::vector<std::string>{});
        for (const auto& it : j.value("patch_plan", nlohmann::json::array())) {
            p.patchPlan.push_back({it.value("intent", ""), it.value("target_file", "")});
        }
        return p;
    }

    static std::string stableId(const PatchProposal& p) {
        std::string key = p.failureClusterId + "|" + p.riskLevel + "|" + std::to_string(p.patchPlan.size());
        uint64_t h = 1099511628211ull;
        for (unsigned char c : key) {
            h ^= c;
            h *= 1469598103934665603ull;
        }
        return "pp_" + std::to_string(h);
    }
};
