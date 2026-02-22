#pragma once
// Step 1521: debug action validator model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugActionValidation {
    bool allowed = true;
    std::vector<std::string> violations;
};

class DebugActionValidator {
public:
    static DebugActionValidation validate(const std::string& action,
                                          bool touchesForbiddenPath,
                                          bool changesManyFiles,
                                          bool bypassesTests) {
        DebugActionValidation v;
        if (action.empty()) {
            v.allowed = false;
            v.violations.push_back("action_empty");
        }
        if (touchesForbiddenPath) {
            v.allowed = false;
            v.violations.push_back("forbidden_path");
        }
        if (changesManyFiles) {
            v.allowed = false;
            v.violations.push_back("change_scope_exceeded");
        }
        if (bypassesTests) {
            v.allowed = false;
            v.violations.push_back("test_bypass_forbidden");
        }
        return v;
    }

    static nlohmann::json toJson(const DebugActionValidation& v) {
        return {{"allowed", v.allowed}, {"violations", v.violations}};
    }
};
