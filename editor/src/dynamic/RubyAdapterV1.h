#pragma once
// Step 752: Ruby lowering adapter v1 (meta-programming boundaries).

#include <nlohmann/json.hpp>

#include "PythonAdapterV2.h"

class RubyLoweringAdapterV1 {
public:
    static DynamicLoweringPacket lower(const std::string& source) {
        DynamicLoweringPacket p;
        p.sourceLanguage = "ruby";
        p.irSummary = source.empty() ? "empty_ruby_unit" : "ruby_ir_v1";
        p.dynamicDispatchCount = source.find("method_missing") != std::string::npos ? 1 : 0;
        p.reviewRequired = p.dynamicDispatchCount > 0;
        return p;
    }

    static nlohmann::json toJson(const DynamicLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary},
                {"dynamic_dispatch_count", p.dynamicDispatchCount}, {"review_required", p.reviewRequired}};
    }
};
