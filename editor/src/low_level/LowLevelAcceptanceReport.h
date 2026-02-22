#pragma once
// Step 807: Low-level acceptance report + waiver system.

#include <nlohmann/json.hpp>

struct LowLevelAcceptanceReport {
    int total = 0;
    int waiverCount = 0;
    bool blocking = false;
    nlohmann::json entries = nlohmann::json::array();
};

struct LowLevelEntry {
    bool reviewRequired = false;
    bool waived = false;
    std::string reason;
};

class LowLevelAcceptanceReportModel {
public:
    static LowLevelAcceptanceReport build(const std::vector<LowLevelEntry>& entries) {
        LowLevelAcceptanceReport report;
        report.total = static_cast<int>(entries.size());
        for (const auto& entry : entries) {
            if (entry.waived) ++report.waiverCount;
            if (entry.reviewRequired && !entry.waived) report.blocking = true;
            report.entries.push_back({{"review_required", entry.reviewRequired}, {"waived", entry.waived}, {"reason", entry.reason}});
        }
        return report;
    }

    static nlohmann::json toJson(const LowLevelAcceptanceReport& report) {
        return {{"total", report.total}, {"waiver_count", report.waiverCount}, {"blocking", report.blocking}, {"entries", report.entries}};
    }
};
