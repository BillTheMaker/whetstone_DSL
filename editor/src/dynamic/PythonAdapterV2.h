#pragma once
// Step 749: Python lowering adapter v2 (dynamic shape packets).

#include <string>

#include <nlohmann/json.hpp>

struct DynamicLoweringPacket {
    std::string sourceLanguage;
    std::string irSummary;
    int dynamicDispatchCount = 0;
    bool reviewRequired = false;
};

class PythonLoweringAdapterV2 {
public:
    static DynamicLoweringPacket lower(const std::string& source) {
        DynamicLoweringPacket p;
        p.sourceLanguage = "python";
        p.irSummary = source.empty() ? "empty_python_unit" : "python_ir_v2";
        p.dynamicDispatchCount = source.find("getattr") != std::string::npos ? 1 : 0;
        p.reviewRequired = p.dynamicDispatchCount > 0;
        return p;
    }

    static nlohmann::json toJson(const DynamicLoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary},
                {"dynamic_dispatch_count", p.dynamicDispatchCount}, {"review_required", p.reviewRequired}};
    }
};
