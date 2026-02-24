#pragma once
// Step 1420: Job-output canonicalization and truncation utilities.
#include <string>
#include <nlohmann/json.hpp>

struct JobOutputCanonicalizationAndTruncationUtilities {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class JobOutputCanonicalizationAndTruncationUtilitiesFactory {
public:
    static JobOutputCanonicalizationAndTruncationUtilities make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const JobOutputCanonicalizationAndTruncationUtilities& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
