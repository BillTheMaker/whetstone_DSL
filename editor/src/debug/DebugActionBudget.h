#pragma once
// Step 1530: debug action budget model.

#include <algorithm>
#include <string>

#include <nlohmann/json.hpp>

struct DebugActionBudget {
    std::string profile = "tiny";
    int maxIterations = 3;
    int maxTokenCost = 4000;
    int maxMinutes = 15;
};

class DebugActionBudgetModel {
public:
    static DebugActionBudget estimate(const std::string& profile,
                                      int failingTargets,
                                      int complexity) {
        DebugActionBudget b;
        b.profile = profile;
        int t = std::max(1, failingTargets);
        int c = std::max(1, complexity);

        if (profile == "tiny") {
            b.maxIterations = std::min(5, t + 1);
            b.maxTokenCost = 2000 + c * 300;
            b.maxMinutes = 10 + t;
        } else if (profile == "small") {
            b.maxIterations = std::min(8, t + 2);
            b.maxTokenCost = 3500 + c * 500;
            b.maxMinutes = 15 + t * 2;
        } else {
            b.maxIterations = std::min(12, t + 4);
            b.maxTokenCost = 5000 + c * 700;
            b.maxMinutes = 20 + t * 3;
        }
        return b;
    }

    static nlohmann::json toJson(const DebugActionBudget& b) {
        return {
            {"profile", b.profile},
            {"max_iterations", b.maxIterations},
            {"max_token_cost", b.maxTokenCost},
            {"max_minutes", b.maxMinutes}
        };
    }
};
