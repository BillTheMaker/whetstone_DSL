#pragma once
// Step 740: Go lowering adapter v1.

#include <nlohmann/json.hpp>

#include "CAdapterV1.h"

class GoLoweringAdapterV1 {
public:
    static LoweringPacket lower(const std::string& source) {
        LoweringPacket p;
        p.sourceLanguage = "go";
        p.irSummary = source.empty() ? "empty_go_unit" : "go_ir_v1";
        p.confidence = source.empty() ? 0.5 : 0.9;
        return p;
    }

    static nlohmann::json toJson(const LoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary}, {"confidence", p.confidence}};
    }
};
