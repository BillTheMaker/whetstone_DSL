#pragma once
// Step 1670: C++ formatting/styling guard model.
#include <string>
#include <nlohmann/json.hpp>

struct CppFormattingStylingGuardModel {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class CppFormattingStylingGuardModelFactory {
public:
    static CppFormattingStylingGuardModel make(const std::string& id,
                        const std::string& detail,
                        int score,
                        bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const CppFormattingStylingGuardModel& v) {
        return {{"id", v.id},
                {"detail", v.detail},
                {"score", v.score},
                {"enabled", v.enabled},
                {"valid", v.valid}};
    }
};
