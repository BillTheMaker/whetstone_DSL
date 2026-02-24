#pragma once
// Step 1686: Reconciliation decision packet model.
#include <string>
#include <nlohmann/json.hpp>

struct ReconciliationDecisionPacketModel {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class ReconciliationDecisionPacketModelFactory {
public:
    static ReconciliationDecisionPacketModel make(const std::string& id,
                        const std::string& detail,
                        int score,
                        bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const ReconciliationDecisionPacketModel& v) {
        return {{"id", v.id},
                {"detail", v.detail},
                {"score", v.score},
                {"enabled", v.enabled},
                {"valid", v.valid}};
    }
};
