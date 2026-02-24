#pragma once
// Step 1696: C++ constructive readiness scorer model.
#include <string>
#include <nlohmann/json.hpp>

struct CppConstructiveReadinessScorerModel {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class CppConstructiveReadinessScorerModelFactory {
public:
    static CppConstructiveReadinessScorerModel make(const std::string& id,
                        const std::string& detail,
                        int score,
                        bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const CppConstructiveReadinessScorerModel& v) {
        return {{"id", v.id},
                {"detail", v.detail},
                {"score", v.score},
                {"enabled", v.enabled},
                {"valid", v.valid}};
    }
};
