#pragma once
// Step 733: baseline-vs-target perf comparator.

#include <nlohmann/json.hpp>

#include "PerfBenchmarkContract.h"

struct PerfComparisonResult {
    double worstRegressionPct = 0.0;
    bool pass = true;
};

class PerfComparator {
public:
    static PerfComparisonResult compare(const std::vector<PerfBenchmarkCase>& cases, double maxAllowedRegressionPct) {
        PerfComparisonResult r;
        for (const auto& c : cases) {
            if (c.baselineMs <= 0.0) continue;
            double pct = ((c.targetMs - c.baselineMs) / c.baselineMs) * 100.0;
            if (pct > r.worstRegressionPct) r.worstRegressionPct = pct;
        }
        r.pass = r.worstRegressionPct <= maxAllowedRegressionPct;
        return r;
    }

    static nlohmann::json toJson(const PerfComparisonResult& r) {
        return {{"worst_regression_pct", r.worstRegressionPct}, {"pass", r.pass}};
    }
};
