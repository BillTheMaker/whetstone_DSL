#pragma once
// Step 644: .whetstone.json for self-hosting editor project

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

using json = nlohmann::json;

struct SelfHostingProjectConfig {
    std::string projectName;
    std::string workspaceRoot;
    std::vector<std::string> sourceRoots;
    std::vector<std::string> testTargets;
    bool selfHosting = false;
};

class SelfHostingProjectConfigModel {
public:
    static json toJson(const SelfHostingProjectConfig& cfg) {
        return {
            {"projectName", cfg.projectName},
            {"workspaceRoot", cfg.workspaceRoot},
            {"sourceRoots", cfg.sourceRoots},
            {"testTargets", cfg.testTargets},
            {"selfHosting", cfg.selfHosting}
        };
    }

    static bool fromJson(const json& j, SelfHostingProjectConfig* out, std::string* error) {
        if (!out || !error) return false;
        error->clear();
        if (!j.is_object()) {
            *error = "config_invalid";
            return false;
        }
        out->projectName = j.value("projectName", "");
        out->workspaceRoot = j.value("workspaceRoot", "");
        out->sourceRoots = j.value("sourceRoots", std::vector<std::string>{});
        out->testTargets = j.value("testTargets", std::vector<std::string>{});
        out->selfHosting = j.value("selfHosting", false);
        if (out->projectName.empty()) {
            *error = "project_name_missing";
            return false;
        }
        return true;
    }

    static bool canSelfHost(const SelfHostingProjectConfig& cfg) {
        return cfg.selfHosting && !cfg.workspaceRoot.empty() && !cfg.sourceRoots.empty();
    }
};
