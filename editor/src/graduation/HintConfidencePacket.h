#pragma once
// Step 862: Hint confidence + uncertainty packet format.
#include <string>
#include <nlohmann/json.hpp>

struct HintConfidencePacket {
    std::string hintId;
    float confidence = 0.0f;
    float uncertainty = 0.0f;
    std::string confidenceBand; // "high", "medium", "low", "unknown"
    bool shouldEscalate = false;
    std::string source;
};

class HintConfidencePacketModel {
public:
    static HintConfidencePacket make(const std::string& hintId,
                                      float confidence, float uncertainty,
                                      const std::string& source) {
        HintConfidencePacket p;
        p.hintId = hintId;
        p.confidence = confidence;
        p.uncertainty = uncertainty;
        p.source = source;
        if (confidence >= 0.8f)      p.confidenceBand = "high";
        else if (confidence >= 0.5f) p.confidenceBand = "medium";
        else if (confidence >= 0.2f) p.confidenceBand = "low";
        else                         p.confidenceBand = "unknown";
        p.shouldEscalate = uncertainty > 0.5f || confidence < 0.3f;
        return p;
    }

    static nlohmann::json toJson(const HintConfidencePacket& p) {
        return {{"hint_id", p.hintId}, {"confidence", p.confidence},
                {"uncertainty", p.uncertainty}, {"band", p.confidenceBand},
                {"should_escalate", p.shouldEscalate}, {"source", p.source}};
    }
};
