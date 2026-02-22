#pragma once
// Step 1469: failure trend tracker model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct FailureTrendPoint {
    int iteration = 0;
    int gapScore = 0;
};

struct FailureTrend {
    std::vector<FailureTrendPoint> points;
    bool worsening = false;
    bool improving = false;
    bool stagnant = false;
};

class FailureTrendTracker {
public:
    static FailureTrend analyze(const std::vector<FailureTrendPoint>& points) {
        FailureTrend t;
        t.points = points;
        if (points.size() < 2) {
            t.stagnant = true;
            return t;
        }
        int delta = points.back().gapScore - points.front().gapScore;
        t.worsening = delta > 0;
        t.improving = delta < 0;
        t.stagnant = delta == 0;
        return t;
    }

    static nlohmann::json toJson(const FailureTrend& t) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& p : t.points) arr.push_back({{"iteration", p.iteration}, {"gap_score", p.gapScore}});
        return {{"points", arr}, {"worsening", t.worsening}, {"improving", t.improving}, {"stagnant", t.stagnant}};
    }
};
