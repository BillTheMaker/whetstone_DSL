#pragma once
// Step 1519: debug recipe step model.

#include <string>

#include <nlohmann/json.hpp>

struct DebugRecipeStep {
    int index = 0;
    std::string action;
    std::string expected;
};

class DebugRecipeStepModel {
public:
    static DebugRecipeStep make(int index, const std::string& action, const std::string& expected) {
        return {index, action, expected};
    }

    static nlohmann::json toJson(const DebugRecipeStep& s) {
        return {{"index", s.index}, {"action", s.action}, {"expected", s.expected}};
    }
};
