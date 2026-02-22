#pragma once
// Step 780: Erlang lowering adapter v1 (actor/mailbox model).

#include <string>

#include <nlohmann/json.hpp>

#include "LogicActorPacketTypes.h"

class ErlangAdapterV1 {
public:
    static LogicActorLoweringPacket lower(const std::string& source) {
        LogicActorLoweringPacket p;
        p.sourceLanguage = "erlang";
        p.irSummary = source.empty() ? "empty_erlang_unit" : "erlang_actor_ir_v1";
        p.actorModel = source.find("spawn") != std::string::npos || source.find("receive") != std::string::npos;
        p.supervision = source.find("supervisor") != std::string::npos;
        return p;
    }

    static nlohmann::json toJson(const LogicActorLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary}, {"actor_model", p.actorModel}, {"supervision", p.supervision}};
    }
};
