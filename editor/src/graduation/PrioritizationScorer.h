#pragma once
// Step 853: Prioritization scoring model (impact x frequency x tier).
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct PrioritizationInput {
    std::string code;
    int frequency = 0;   // occurrences
    int impact = 0;      // 1-10
    int tierWeight = 1;  // stable=3, beta=2, experimental=1
};

struct PrioritizationScore {
    std::string code;
    int score = 0;
    std::string priority; // "critical", "high", "medium", "low"
};

class PrioritizationScorer {
public:
    static PrioritizationScore score(const PrioritizationInput& in) {
        PrioritizationScore s;
        s.code = in.code;
        s.score = in.frequency * in.impact * in.tierWeight;
        if (s.score >= 100)     s.priority = "critical";
        else if (s.score >= 40) s.priority = "high";
        else if (s.score >= 10) s.priority = "medium";
        else                    s.priority = "low";
        return s;
    }

    static std::vector<PrioritizationScore> rank(std::vector<PrioritizationScore> scores) {
        std::sort(scores.begin(), scores.end(),
            [](const PrioritizationScore& a, const PrioritizationScore& b) {
                return a.score > b.score;
            });
        return scores;
    }

    static nlohmann::json toJson(const PrioritizationScore& s) {
        return {{"code", s.code}, {"score", s.score}, {"priority", s.priority}};
    }
};
