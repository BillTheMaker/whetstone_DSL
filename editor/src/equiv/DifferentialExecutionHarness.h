#pragma once
// Step 719: differential execution harness core.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DifferentialCaseResult {
    std::string caseId;
    std::string sourceOutput;
    std::string targetOutput;
    bool equivalent = false;
};

struct DifferentialRunResult {
    std::vector<DifferentialCaseResult> cases;
    int equivalentCount = 0;
    int totalCount = 0;
    double confidence = 0.0;
};

class DifferentialExecutionHarness {
public:
    static DifferentialRunResult run(const std::vector<DifferentialCaseResult>& inputCases) {
        DifferentialRunResult out;
        out.cases = inputCases;
        std::sort(out.cases.begin(), out.cases.end(), [](const DifferentialCaseResult& a, const DifferentialCaseResult& b) {
            return a.caseId < b.caseId;
        });
        out.totalCount = static_cast<int>(out.cases.size());
        for (auto& c : out.cases) {
            c.equivalent = c.sourceOutput == c.targetOutput;
            if (c.equivalent) ++out.equivalentCount;
        }
        out.confidence = out.totalCount == 0 ? 0.0 : static_cast<double>(out.equivalentCount) / out.totalCount;
        return out;
    }

    static nlohmann::json toJson(const DifferentialRunResult& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& c : r.cases) {
            arr.push_back({
                {"case_id", c.caseId},
                {"source_output", c.sourceOutput},
                {"target_output", c.targetOutput},
                {"equivalent", c.equivalent}
            });
        }
        return {
            {"cases", arr},
            {"equivalent_count", r.equivalentCount},
            {"total_count", r.totalCount},
            {"confidence", r.confidence}
        };
    }
};
