#pragma once
// Step 787: Family acceptance and divergence report.

#include <nlohmann/json.hpp>

#include "SemanticBlocklistGate.h"

struct LogicActorAcceptanceReport {
    int total = 0;
    int blocked = 0;
    int reviewRequired = 0;
    nlohmann::json items = nlohmann::json::array();
};

class LogicActorAcceptanceReportModel {
public:
    static LogicActorAcceptanceReport build(const std::vector<SemanticGatePacket>& gates) {
        LogicActorAcceptanceReport out;
        out.total = static_cast<int>(gates.size());
        for (size_t i = 0; i < gates.size(); ++i) {
            if (gates[i].blocked) ++out.blocked;
            if (gates[i].reviewRequired) ++out.reviewRequired;
            out.items.push_back(SemanticBlocklistGate::toJson(gates[i]));
        }
        return out;
    }

    static nlohmann::json toJson(const LogicActorAcceptanceReport& r) {
        return {{"total", r.total}, {"blocked", r.blocked}, {"review_required", r.reviewRequired}, {"items", r.items}};
    }
};
