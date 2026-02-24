#pragma once
// Step 1151: Third-party conformance replay runner.
#include <string>
#include <nlohmann/json.hpp>

struct ThirdPartyConformanceReplayRunner {
    std::string runId;
    std::string vendorId;
    int scenarioCount = 0;
    int passedCount = 0;
    double passRate = 0.0;
    bool conformant = false;
};

class ThirdPartyConformanceReplayRunnerFactory {
public:
    static ThirdPartyConformanceReplayRunner make(const std::string& runId,
                                                  const std::string& vendorId,
                                                  int scenarioCount,
                                                  int passedCount) {
        int safeScenarios = scenarioCount < 0 ? 0 : scenarioCount;
        int safePassed = passedCount < 0 ? 0 : passedCount;
        if (safePassed > safeScenarios) safePassed = safeScenarios;
        double passRate = safeScenarios == 0 ? 0.0 : static_cast<double>(safePassed) / safeScenarios;
        bool conformant = safeScenarios > 0 && passRate >= 0.90;
        return {runId, vendorId, safeScenarios, safePassed, passRate, conformant};
    }

    static nlohmann::json toJson(const ThirdPartyConformanceReplayRunner& r) {
        return {{"run_id", r.runId},
                {"vendor_id", r.vendorId},
                {"scenario_count", r.scenarioCount},
                {"passed_count", r.passedCount},
                {"pass_rate", r.passRate},
                {"conformant", r.conformant}};
    }
};
