#pragma once
// Step 795: Null/join/aggregation divergence classifier.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SqlCanonicalQueryIR.h"

struct QueryDivergencePacket {
    std::string level;
    std::vector<std::string> reasons;
};

class QueryDivergenceClassifier {
public:
    static QueryDivergencePacket classify(const QueryLoweringPacket& p) {
        QueryDivergencePacket out;
        int score = 0;
        if (p.hasJoin) { score += 1; out.reasons.push_back("join_semantics"); }
        if (p.hasAggregate) { score += 1; out.reasons.push_back("aggregate_semantics"); }
        if (p.hasNullSemantics) { score += 1; out.reasons.push_back("null_semantics"); }
        if (score == 0) out.level = "low";
        else if (score == 1) out.level = "medium";
        else out.level = "high";
        if (out.reasons.empty()) out.reasons.push_back("none");
        return out;
    }

    static nlohmann::json toJson(const QueryDivergencePacket& p) {
        return {{"level", p.level}, {"reasons", p.reasons}};
    }
};
