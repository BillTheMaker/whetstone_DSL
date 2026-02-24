#pragma once
// Step 1214: Verification gate integration for architecture artifacts.
#include <string>
#include <nlohmann/json.hpp>

struct VerificationGateIntegrationForArchitectureArtifacts {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class VerificationGateIntegrationForArchitectureArtifactsFactory {
public:
    static VerificationGateIntegrationForArchitectureArtifacts make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const VerificationGateIntegrationForArchitectureArtifacts& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
