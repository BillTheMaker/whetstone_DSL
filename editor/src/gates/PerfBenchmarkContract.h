#pragma once
// Step 732: perf benchmark harness contract.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct PerfBenchmarkCase {
    std::string name;
    double baselineMs = 0.0;
    double targetMs = 0.0;
};

class PerfBenchmarkContract {
public:
    static std::vector<PerfBenchmarkCase> normalize(std::vector<PerfBenchmarkCase> cases) {
        std::sort(cases.begin(), cases.end(), [](const PerfBenchmarkCase& a, const PerfBenchmarkCase& b) {
            return a.name < b.name;
        });
        return cases;
    }

    static nlohmann::json toJson(const std::vector<PerfBenchmarkCase>& cases) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& c : cases) arr.push_back({{"name", c.name}, {"baseline_ms", c.baselineMs}, {"target_ms", c.targetMs}});
        return arr;
    }
};
