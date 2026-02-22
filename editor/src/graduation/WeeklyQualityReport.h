#pragma once
// Step 857: Weekly quality report artifact generator.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct WeeklyQualityReport {
    std::string reportId;
    std::string weekPeriod;
    int totalFailures = 0;
    int newFailures = 0;
    int resolvedFailures = 0;
    std::vector<std::string> topCodes;
    bool improved = false;
};

class WeeklyQualityReportGenerator {
public:
    static WeeklyQualityReport generate(const std::string& reportId,
                                         const std::string& period,
                                         int total, int newF, int resolved,
                                         const std::vector<std::string>& topCodes) {
        WeeklyQualityReport r;
        r.reportId = reportId;
        r.weekPeriod = period;
        r.totalFailures = total;
        r.newFailures = newF;
        r.resolvedFailures = resolved;
        r.topCodes = topCodes;
        r.improved = resolved > newF;
        return r;
    }

    static nlohmann::json toJson(const WeeklyQualityReport& r) {
        nlohmann::json codes = nlohmann::json::array();
        for (const auto& c : r.topCodes) codes.push_back(c);
        return {{"report_id", r.reportId}, {"period", r.weekPeriod},
                {"total_failures", r.totalFailures}, {"new_failures", r.newFailures},
                {"resolved", r.resolvedFailures}, {"improved", r.improved},
                {"top_codes", codes}};
    }
};
