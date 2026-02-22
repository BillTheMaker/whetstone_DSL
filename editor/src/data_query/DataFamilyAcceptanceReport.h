#pragma once
// Step 797: Data family acceptance report.

#include <nlohmann/json.hpp>

#include "QueryDivergenceClassifier.h"

struct DataFamilyAcceptanceReport {
    int total = 0;
    int highRisk = 0;
    int mediumRisk = 0;
    nlohmann::json items = nlohmann::json::array();
};

class DataFamilyAcceptanceReportModel {
public:
    static DataFamilyAcceptanceReport build(const std::vector<QueryDivergencePacket>& packets) {
        DataFamilyAcceptanceReport r;
        r.total = static_cast<int>(packets.size());
        for (const auto& p : packets) {
            if (p.level == "high") ++r.highRisk;
            if (p.level == "medium") ++r.mediumRisk;
            r.items.push_back(QueryDivergenceClassifier::toJson(p));
        }
        return r;
    }

    static nlohmann::json toJson(const DataFamilyAcceptanceReport& r) {
        return {{"total", r.total}, {"high_risk", r.highRisk}, {"medium_risk", r.mediumRisk}, {"items", r.items}};
    }
};
