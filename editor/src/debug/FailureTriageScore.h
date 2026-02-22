#pragma once
// Step 1510: failure triage score model.

#include <algorithm>
#include <string>

#include <nlohmann/json.hpp>

struct FailureTriageScore {
    std::string failureClass;
    int severity = 0;
    int reproducibility = 0;
    int blastRadius = 0;
    int score = 0;
    std::string tier = "low";
};

class FailureTriageScorer {
public:
    static FailureTriageScore compute(const std::string& failureClass,
                                      int severity,
                                      int reproducibility,
                                      int blastRadius) {
        FailureTriageScore s;
        s.failureClass = failureClass;
        s.severity = clamp10(severity);
        s.reproducibility = clamp10(reproducibility);
        s.blastRadius = clamp10(blastRadius);
        s.score = s.severity * 5 + s.reproducibility * 3 + s.blastRadius * 2;
        s.tier = tierFor(s.score);
        return s;
    }

    static nlohmann::json toJson(const FailureTriageScore& s) {
        return {
            {"failure_class", s.failureClass},
            {"severity", s.severity},
            {"reproducibility", s.reproducibility},
            {"blast_radius", s.blastRadius},
            {"score", s.score},
            {"tier", s.tier}
        };
    }

private:
    static int clamp10(int v) { return std::max(0, std::min(10, v)); }

    static std::string tierFor(int score) {
        if (score >= 70) return "critical";
        if (score >= 40) return "high";
        if (score >= 20) return "medium";
        return "low";
    }
};
