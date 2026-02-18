#pragma once
// Step 660: whetstone_generate_inference_job MCP tool model

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

using json = nlohmann::json;

class InferenceJobGenerator {
public:
    static std::string toolName() { return "whetstone_generate_inference_job"; }

    static json generate(const std::string& goal,
                         int entropyScore,
                         const std::vector<std::string>& files,
                         const std::string& bounty = "normal") {
        if (goal.empty() || entropyScore < 0) {
            return {{"success", false}, {"error", "invalid_input"}};
        }
        return {
            {"success", true},
            {"job", {
                {"type", "refactor"},
                {"goal", goal},
                {"context", {{"files", files}, {"entropy_score", entropyScore}}},
                {"bounty", bounty}
            }}
        };
    }
};
