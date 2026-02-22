#pragma once
// Step 753: Lua lowering adapter v1 (table/closure semantics).

#include <nlohmann/json.hpp>

#include "PythonAdapterV2.h"

class LuaLoweringAdapterV1 {
public:
    static DynamicLoweringPacket lower(const std::string& source) {
        DynamicLoweringPacket p;
        p.sourceLanguage = "lua";
        p.irSummary = source.empty() ? "empty_lua_unit" : "lua_ir_v1";
        p.dynamicDispatchCount = source.find("setmetatable") != std::string::npos ? 1 : 0;
        p.reviewRequired = p.dynamicDispatchCount > 0;
        return p;
    }

    static nlohmann::json toJson(const DynamicLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary},
                {"dynamic_dispatch_count", p.dynamicDispatchCount}, {"review_required", p.reviewRequired}};
    }
};
