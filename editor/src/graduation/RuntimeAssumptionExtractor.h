#pragma once
// Step 891: Runtime assumption extraction from source projects.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "RuntimeSemanticsPack.h"

struct ExtractedRuntimeHint {
    std::string hintId;
    std::string runtimeId;
    std::string category;
    std::string evidence;   // what triggered this hint
    float confidence = 0.0f;
};

class RuntimeAssumptionExtractor {
public:
    static std::vector<ExtractedRuntimeHint> extract(
            const std::string& runtimeId,
            const std::vector<std::string>& sourcePatterns) {
        std::vector<ExtractedRuntimeHint> hints;
        int idx = 0;
        for (const auto& pat : sourcePatterns) {
            ExtractedRuntimeHint h;
            h.hintId = "RH-" + std::to_string(++idx);
            h.runtimeId = runtimeId;
            h.category = "lifecycle";
            h.evidence = pat;
            h.confidence = 0.7f;
            hints.push_back(h);
        }
        return hints;
    }

    static nlohmann::json toJson(const ExtractedRuntimeHint& h) {
        return {{"hint_id", h.hintId}, {"runtime_id", h.runtimeId},
                {"category", h.category}, {"evidence", h.evidence},
                {"confidence", h.confidence}};
    }
};
