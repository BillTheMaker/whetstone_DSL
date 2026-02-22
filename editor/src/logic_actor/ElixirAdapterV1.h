#pragma once
// Step 781: Elixir lowering adapter v1 (actor + macro surface model).

#include <string>

#include <nlohmann/json.hpp>

#include "LogicActorPacketTypes.h"

class ElixirAdapterV1 {
public:
    static LogicActorLoweringPacket lower(const std::string& source) {
        LogicActorLoweringPacket p;
        p.sourceLanguage = "elixir";
        p.irSummary = source.empty() ? "empty_elixir_unit" : "elixir_actor_ir_v1";
        p.actorModel = source.find("spawn") != std::string::npos || source.find("receive") != std::string::npos;
        p.supervision = source.find("Supervisor") != std::string::npos;
        p.macroSurface = source.find("defmacro") != std::string::npos;
        return p;
    }

    static nlohmann::json toJson(const LogicActorLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary}, {"actor_model", p.actorModel}, {"supervision", p.supervision}, {"macro_surface", p.macroSurface}};
    }
};
