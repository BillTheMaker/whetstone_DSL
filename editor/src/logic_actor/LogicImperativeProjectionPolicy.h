#pragma once
// Step 782: Logic-to-imperative projection policy set.

#include <string>

#include <nlohmann/json.hpp>

#include "LogicActorPacketTypes.h"

struct LogicProjectionPolicyPacket {
    std::string policy;
    bool preservesBacktracking = false;
    bool reviewRequired = false;
};

class LogicImperativeProjectionPolicy {
public:
    static LogicProjectionPolicyPacket choose(const LogicActorLoweringPacket& p, const std::string& targetLanguage) {
        LogicProjectionPolicyPacket out;
        out.policy = targetLanguage + "_logic_projection";
        out.preservesBacktracking = (targetLanguage == "prolog");
        out.reviewRequired = p.backtracking && !out.preservesBacktracking;
        return out;
    }

    static nlohmann::json toJson(const LogicProjectionPolicyPacket& p) {
        return {{"policy", p.policy}, {"preserves_backtracking", p.preservesBacktracking}, {"review_required", p.reviewRequired}};
    }
};
