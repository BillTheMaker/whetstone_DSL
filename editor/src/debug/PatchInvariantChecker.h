#pragma once
// Step 1461: patch invariant checker.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "PatchProposal.h"

struct InvariantCheckResult {
    bool success = false;
    std::vector<std::string> violations;
};

class PatchInvariantChecker {
public:
    static InvariantCheckResult check(const PatchProposal& p,
                                      const std::vector<std::string>& allowedFiles) {
        InvariantCheckResult r;

        if (p.invariants.empty()) r.violations.push_back("invariants_missing");
        if (p.expectedTestsToRun.empty()) r.violations.push_back("expected_tests_missing");

        std::string err;
        if (!PatchProposalModel::validate(p, allowedFiles, &err)) r.violations.push_back(err);

        bool hasNoUnrelated = false;
        for (const auto& i : p.invariants) if (i == "do_not_edit_unrelated_files") hasNoUnrelated = true;
        if (!hasNoUnrelated) r.violations.push_back("scope_invariant_missing");

        r.success = r.violations.empty();
        return r;
    }

    static nlohmann::json toJson(const InvariantCheckResult& r) {
        return {{"success", r.success}, {"violations", r.violations}};
    }
};
