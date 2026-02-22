#pragma once
// Step 1491: safety envelope policy.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct SafetyEnvelope {
    int maxFailures = 3;
    int maxRegressions = 1;
    int maxTokensPerCampaign = 20000;
};

struct SafetyEnvelopeResult {
    bool allowed = true;
    std::vector<std::string> violations;
};

class SafetyEnvelopePolicy {
public:
    static SafetyEnvelopeResult evaluate(const SafetyEnvelope& e,
                                         int failures,
                                         int regressions,
                                         int tokenCost) {
        SafetyEnvelopeResult r;
        if (failures > e.maxFailures) r.violations.push_back("max_failures_exceeded");
        if (regressions > e.maxRegressions) r.violations.push_back("max_regressions_exceeded");
        if (tokenCost > e.maxTokensPerCampaign) r.violations.push_back("max_tokens_exceeded");
        r.allowed = r.violations.empty();
        return r;
    }

    static nlohmann::json toJson(const SafetyEnvelopeResult& r) {
        return {{"allowed", r.allowed}, {"violations", r.violations}};
    }
};
