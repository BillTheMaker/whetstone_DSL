#pragma once
// Step 765: ADT + pattern matching canonical lowering.

#include <string>

#include <nlohmann/json.hpp>

struct ADTPatternPacket {
    bool adtDetected = false;
    bool patternMatchDetected = false;
    std::string canonicalShape;
};

class ADTPatternCanonicalLowering {
public:
    static ADTPatternPacket lower(const std::string& source) {
        ADTPatternPacket p;
        p.adtDetected = source.find("sealed") != std::string::npos ||
                        source.find("record") != std::string::npos ||
                        source.find("type") != std::string::npos;
        p.patternMatchDetected = source.find("when") != std::string::npos ||
                                 source.find("match") != std::string::npos ||
                                 source.find("switch") != std::string::npos ||
                                 source.find("Select Case") != std::string::npos;
        if (p.adtDetected && p.patternMatchDetected) p.canonicalShape = "sum_type_with_patterns";
        else if (p.adtDetected) p.canonicalShape = "sum_type";
        else p.canonicalShape = "nominal";
        return p;
    }

    static nlohmann::json toJson(const ADTPatternPacket& p) {
        return {
            {"adt_detected", p.adtDetected},
            {"pattern_match_detected", p.patternMatchDetected},
            {"canonical_shape", p.canonicalShape}
        };
    }
};
