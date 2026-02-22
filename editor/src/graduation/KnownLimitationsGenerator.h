#pragma once
// Step 831: Pair-specific known-limitations generator.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct KnownLimitation {
    std::string limitationId;
    std::string pairId;
    std::string category; // "semantics", "performance", "syntax", "runtime"
    std::string description;
    std::string workaround;
    std::string severity; // "low", "medium", "high"
};

struct LimitationsBundle {
    std::string pairId;
    std::vector<KnownLimitation> limitations;
    int highCount = 0;
    int mediumCount = 0;
    int lowCount = 0;
};

class KnownLimitationsGenerator {
public:
    static LimitationsBundle generate(const std::string& pairId,
                                      const std::vector<KnownLimitation>& items) {
        LimitationsBundle b;
        b.pairId = pairId;
        b.limitations = items;
        for (const auto& l : items) {
            if (l.severity == "high")        ++b.highCount;
            else if (l.severity == "medium") ++b.mediumCount;
            else                             ++b.lowCount;
        }
        return b;
    }

    static nlohmann::json toJson(const LimitationsBundle& b) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& l : b.limitations)
            arr.push_back({{"id", l.limitationId}, {"category", l.category},
                           {"description", l.description}, {"severity", l.severity}});
        return {{"pair_id", b.pairId}, {"limitations", arr},
                {"high", b.highCount}, {"medium", b.mediumCount}, {"low", b.lowCount}};
    }
};
