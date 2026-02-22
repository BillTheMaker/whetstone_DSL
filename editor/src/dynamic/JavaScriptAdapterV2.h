#pragma once
// Step 750: JavaScript lowering adapter v2 (prototype/object model).

#include <nlohmann/json.hpp>

#include "PythonAdapterV2.h"

class JavaScriptLoweringAdapterV2 {
public:
    static DynamicLoweringPacket lower(const std::string& source) {
        DynamicLoweringPacket p;
        p.sourceLanguage = "javascript";
        p.irSummary = source.empty() ? "empty_js_unit" : "js_ir_v2";
        p.dynamicDispatchCount = source.find("[") != std::string::npos ? 1 : 0;
        p.reviewRequired = p.dynamicDispatchCount > 0;
        return p;
    }

    static nlohmann::json toJson(const DynamicLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary},
                {"dynamic_dispatch_count", p.dynamicDispatchCount}, {"review_required", p.reviewRequired}};
    }
};
