#pragma once
// Step 552: Cost vs Quality Regression Suite

#include <string>
#include <vector>

struct CostQualityRun {
    std::string profile;
    int tokensUsed = 0;
    double qualityScore = 0.0; // 0..1
    double passRate = 0.0;     // 0..1
    int safetyIncidents = 0;
};

struct CostQualityPolicy {
    double minQualityScore = 0.0;
    double minPassRate = 0.0;
    int maxSafetyIncidents = 0;
    int minTokenReduction = 0;
};

struct CostQualityRegressionResult {
    bool pass = false;
    int tokenReduction = 0;
    std::vector<std::string> failures;
};

class CostQualityRegressionSuite {
public:
    static CostQualityRegressionResult evaluate(const CostQualityRun& baseline,
                                                const CostQualityRun& optimized,
                                                const CostQualityPolicy& policy) {
        CostQualityRegressionResult out;
        out.tokenReduction = baseline.tokensUsed - optimized.tokensUsed;

        if (out.tokenReduction < policy.minTokenReduction) {
            out.failures.push_back("insufficient_token_reduction");
        }
        if (optimized.qualityScore < policy.minQualityScore) {
            out.failures.push_back("quality_below_floor");
        }
        if (optimized.passRate < policy.minPassRate) {
            out.failures.push_back("pass_rate_below_floor");
        }
        if (optimized.safetyIncidents > policy.maxSafetyIncidents) {
            out.failures.push_back("safety_incidents_exceeded");
        }

        // Relative regression guard: avoid hidden quality collapse even if absolute floor passed.
        if (optimized.qualityScore + 1e-9 < baseline.qualityScore - 0.08) {
            out.failures.push_back("relative_quality_regression");
        }
        if (optimized.passRate + 1e-9 < baseline.passRate - 0.08) {
            out.failures.push_back("relative_pass_rate_regression");
        }

        out.pass = out.failures.empty();
        return out;
    }
};
