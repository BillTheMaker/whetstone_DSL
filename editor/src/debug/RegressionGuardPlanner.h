#pragma once
// Step 1455: deterministic regression guard planner.

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RegressionGuardPlan {
    std::vector<std::string> mustPass;
    std::vector<std::string> optional;
    std::vector<std::string> deferred;
};

class RegressionGuardPlanner {
public:
    static RegressionGuardPlan plan(const std::vector<std::string>& touchedFiles,
                                    int stepId,
                                    const std::string& failingTarget) {
        RegressionGuardPlan p;
        if (!failingTarget.empty()) p.mustPass.push_back(failingTarget);

        if (stepId > 1) p.mustPass.push_back("step" + std::to_string(stepId - 1) + "_test");
        p.mustPass.push_back("step" + std::to_string(stepId) + "_test");
        p.mustPass.push_back("step" + std::to_string(stepId + 1) + "_test");

        bool mcpTouched = false;
        for (const auto& f : touchedFiles) if (f.find("MCP") != std::string::npos || f.find("mcp/") != std::string::npos) mcpTouched = true;
        if (mcpTouched) p.mustPass.push_back("mcp_tools_smoke_test");

        p.optional = {"lint_smoke", "format_smoke"};
        p.deferred = {"full_regression_suite"};

        dedupSort(&p.mustPass);
        dedupSort(&p.optional);
        dedupSort(&p.deferred);
        return p;
    }

    static nlohmann::json toJson(const RegressionGuardPlan& p) {
        return {{"must_pass", p.mustPass}, {"optional", p.optional}, {"deferred", p.deferred}};
    }

private:
    static void dedupSort(std::vector<std::string>* xs) {
        if (!xs) return;
        std::sort(xs->begin(), xs->end());
        xs->erase(std::unique(xs->begin(), xs->end()), xs->end());
    }
};
