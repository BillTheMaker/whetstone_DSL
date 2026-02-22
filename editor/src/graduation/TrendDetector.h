#pragma once
// Step 852: Trend and recurrence detector.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct TrendDataPoint {
    std::string period;  // "2026-W01"
    std::string code;
    int count = 0;
};

struct TrendResult {
    std::string code;
    bool recurring = false;
    bool increasing = false;
    int periods = 0;
    int peakCount = 0;
};

class TrendDetector {
public:
    static TrendResult detect(const std::string& code,
                               const std::vector<TrendDataPoint>& points) {
        TrendResult r;
        r.code = code;
        r.periods = 0;
        r.peakCount = 0;
        std::vector<int> counts;
        for (const auto& p : points) {
            if (p.code == code) {
                ++r.periods;
                counts.push_back(p.count);
                if (p.count > r.peakCount) r.peakCount = p.count;
            }
        }
        r.recurring = r.periods >= 2;
        if (counts.size() >= 2)
            r.increasing = counts.back() > counts.front();
        return r;
    }

    static nlohmann::json toJson(const TrendResult& r) {
        return {{"code", r.code}, {"recurring", r.recurring},
                {"increasing", r.increasing}, {"periods", r.periods},
                {"peak_count", r.peakCount}};
    }
};
