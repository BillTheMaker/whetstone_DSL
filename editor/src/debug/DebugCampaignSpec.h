#pragma once
// Step 1489: debug campaign spec model.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugCampaignSpec {
    std::string campaignId;
    std::string name;
    std::vector<std::string> targets;
    std::string budgetMode = "tiny";
    int maxIterationsPerTarget = 3;
    bool applyPatches = true;
};

class DebugCampaignSpecModel {
public:
    static DebugCampaignSpec make(const std::string& name,
                                  std::vector<std::string> targets,
                                  const std::string& budgetMode,
                                  int maxIter,
                                  bool apply) {
        DebugCampaignSpec s;
        s.name = name;
        std::sort(targets.begin(), targets.end());
        s.targets = std::move(targets);
        s.budgetMode = budgetMode;
        s.maxIterationsPerTarget = maxIter;
        s.applyPatches = apply;
        s.campaignId = stableId(s);
        return s;
    }

    static nlohmann::json toJson(const DebugCampaignSpec& s) {
        return {{"campaign_id", s.campaignId}, {"name", s.name}, {"targets", s.targets},
                {"budget_mode", s.budgetMode}, {"max_iterations_per_target", s.maxIterationsPerTarget},
                {"apply_patches", s.applyPatches}};
    }

private:
    static std::string stableId(const DebugCampaignSpec& s) {
        uint64_t h = 1469598103934665603ull;
        std::string key = s.name + "|" + s.budgetMode + "|" + std::to_string(s.maxIterationsPerTarget);
        for (const auto& t : s.targets) key += "|" + t;
        for (unsigned char c : key) { h ^= c; h *= 1099511628211ull; }
        return "dc_" + std::to_string(h);
    }
};
