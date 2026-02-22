#pragma once
// Step 739: C lowering adapter v1.

#include <string>

#include <nlohmann/json.hpp>

struct LoweringPacket {
    std::string sourceLanguage;
    std::string irSummary;
    double confidence = 0.0;
};

class CLoweringAdapterV1 {
public:
    static LoweringPacket lower(const std::string& source) {
        LoweringPacket p;
        p.sourceLanguage = "c";
        p.irSummary = source.empty() ? "empty_c_unit" : "c_ir_v1";
        p.confidence = source.empty() ? 0.5 : 0.9;
        return p;
    }

    static nlohmann::json toJson(const LoweringPacket& p) {
        return {{"source_language", p.sourceLanguage}, {"ir_summary", p.irSummary}, {"confidence", p.confidence}};
    }
};
