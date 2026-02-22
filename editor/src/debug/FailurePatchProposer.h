#pragma once
// Step 1453: failure-class-driven patch proposer.

#include <string>
#include <vector>

#include "FailureClusterer.h"
#include "FixContextAssembler.h"
#include "PatchProposal.h"

struct FailurePatchSuggestion {
    PatchProposal proposal;
    double confidence = 0.0;
    std::string rationale;
};

class FailurePatchProposer {
public:
    static FailurePatchSuggestion propose(const FailureCluster& cluster,
                                          const FixContextPacket& context,
                                          const std::string& testTarget = "") {
        FailurePatchSuggestion s;
        s.proposal.failureClusterId = cluster.clusterId;

        std::string target = context.slices.empty() ? "editor/src/unknown.h" : context.slices[0].path;
        std::string strategy = strategyFor(cluster.failureClass);
        s.rationale = "strategy=" + strategy + " class=" + cluster.failureClass;

        s.proposal.patchPlan.push_back({strategy, target});
        s.proposal.expectedTestsToRun = testTarget.empty() ? std::vector<std::string>{"step_smoke_test"}
                                                           : std::vector<std::string>{testTarget};
        s.proposal.invariants = {
            "do_not_edit_unrelated_files",
            "preserve_schema_keys",
            "keep_output_deterministic"
        };
        s.proposal.riskLevel = (cluster.failureClass == "compile_error") ? "low" : "medium";

        s.proposal.candidateDiff =
            "diff --git a/" + target + " b/" + target + "\n"
            "--- a/" + target + "\n"
            "+++ b/" + target + "\n"
            "@@\n"
            "+// auto-fix placeholder for " + strategy + "\n";

        s.confidence = confidenceFor(cluster.failureClass);
        s.proposal.proposalId = PatchProposalModel::stableId(s.proposal);
        return s;
    }

private:
    static std::string strategyFor(const std::string& failureClass) {
        if (failureClass == "compile_error") return "missing_include_or_symbol_fix";
        if (failureClass == "schema_error") return "schema_key_alignment_fix";
        if (failureClass == "test_assertion") return "assertion_behavior_adjustment_fix";
        if (failureClass == "tool_contract_error") return "tool_input_contract_fix";
        return "safe_fallback_review_fix";
    }

    static double confidenceFor(const std::string& failureClass) {
        if (failureClass == "compile_error") return 0.85;
        if (failureClass == "schema_error") return 0.8;
        if (failureClass == "test_assertion") return 0.75;
        if (failureClass == "tool_contract_error") return 0.78;
        return 0.4;
    }
};
