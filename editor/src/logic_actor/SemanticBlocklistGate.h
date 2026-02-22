#pragma once
// Step 785: Unmappable semantic blocklist + mandatory review gate.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "LogicActorPacketTypes.h"

struct SemanticGatePacket {
    bool blocked = false;
    bool reviewRequired = false;
    std::vector<std::string> reasons;
};

class SemanticBlocklistGate {
public:
    static SemanticGatePacket evaluate(const LogicActorLoweringPacket& p, const std::string& targetLanguage) {
        SemanticGatePacket out;
        if (p.actorModel && targetLanguage == "c") {
            out.blocked = true;
            out.reasons.push_back("actor_mailbox_unmappable_to_c_runtime");
        }
        if (p.backtracking && targetLanguage != "prolog") {
            out.reasons.push_back("backtracking_semantics_loss_risk");
        }
        out.reviewRequired = out.blocked || !out.reasons.empty();
        if (out.reasons.empty()) out.reasons.push_back("no_blockers");
        return out;
    }

    static nlohmann::json toJson(const SemanticGatePacket& p) {
        return {{"blocked", p.blocked}, {"review_required", p.reviewRequired}, {"reasons", p.reasons}};
    }
};
