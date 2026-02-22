#pragma once
// Step 751: TypeScript lowering adapter v2 (declared type integration).

#include <nlohmann/json.hpp>

#include "PythonAdapterV2.h"

class TypeScriptLoweringAdapterV2 {
public:
    static DynamicLoweringPacket lower(const std::string& source) {
        DynamicLoweringPacket p;
        p.sourceLanguage = "typescript";
        p.irSummary = source.empty() ? "empty_ts_unit" : "ts_ir_v2";
        p.dynamicDispatchCount = source.find("any") != std::string::npos ? 1 : 0;
        p.reviewRequired = p.dynamicDispatchCount > 0;
        return p;
    }

    static nlohmann::json toJson(const DynamicLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary},
                {"dynamic_dispatch_count", p.dynamicDispatchCount}, {"review_required", p.reviewRequired}};
    }
};
