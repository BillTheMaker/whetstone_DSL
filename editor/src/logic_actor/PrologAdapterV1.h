#pragma once
// Step 779: Prolog lowering adapter v1 (query/backtracking model).

#include <string>

#include <nlohmann/json.hpp>

#include "LogicActorPacketTypes.h"

class PrologAdapterV1 {
public:
    static LogicActorLoweringPacket lower(const std::string& source) {
        LogicActorLoweringPacket p;
        p.sourceLanguage = "prolog";
        p.irSummary = source.empty() ? "empty_prolog_unit" : "prolog_logic_ir_v1";
        p.queryArity = source.find("(") != std::string::npos ? 1 : 0;
        p.backtracking = source.find(";") != std::string::npos || source.find("fail") != std::string::npos;
        return p;
    }

    static nlohmann::json toJson(const LogicActorLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary}, {"query_arity", p.queryArity}, {"backtracking", p.backtracking}};
    }
};
