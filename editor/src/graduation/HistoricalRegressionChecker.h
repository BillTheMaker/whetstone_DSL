#pragma once
// Step 838: Full historical regression + post-release checklist.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct RegressionCheckItem {
    std::string checkId;
    std::string description;
    bool passed = false;
};

struct HistoricalRegressionReport {
    std::vector<RegressionCheckItem> items;
    int totalChecks = 0;
    int passedChecks = 0;
    bool clean = false;
};

class HistoricalRegressionChecker {
public:
    static HistoricalRegressionReport run(std::vector<RegressionCheckItem> items) {
        HistoricalRegressionReport r;
        r.items = items;
        r.totalChecks = static_cast<int>(items.size());
        for (const auto& i : items) if (i.passed) ++r.passedChecks;
        r.clean = r.passedChecks == r.totalChecks && r.totalChecks > 0;
        return r;
    }

    static nlohmann::json toJson(const HistoricalRegressionReport& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& i : r.items)
            arr.push_back({{"check_id", i.checkId}, {"passed", i.passed}});
        return {{"total", r.totalChecks}, {"passed", r.passedChecks},
                {"clean", r.clean}, {"items", arr}};
    }
};
