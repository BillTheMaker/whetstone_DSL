#pragma once
// Step 815: Legacy->IR migration readiness scoring.

#include <string>

#include <nlohmann/json.hpp>

struct MigrationScore {
    double completeness = 0.0;
    double confidence = 0.0;
    bool ready = false;
};

class MigrationReadiness {
public:
    static MigrationScore evaluate(double confidence, size_t nodes) {
        MigrationScore score;
        score.confidence = confidence;
        score.completeness = std::min(1.0, nodes / 5.0);
        score.ready = (score.completeness > 0.5 && score.confidence > 0.6);
        return score;
    }

    static nlohmann::json toJson(const MigrationScore& s) {
        return {{"completeness", s.completeness}, {"confidence", s.confidence}, {"ready", s.ready}};
    }
};
