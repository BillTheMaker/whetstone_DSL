#pragma once
// Step 1520: debug recipe library model.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "DebugRecipeStep.h"

struct DebugRecipe {
    std::string failureClass;
    std::vector<DebugRecipeStep> steps;
};

class DebugRecipeLibrary {
public:
    static DebugRecipe get(const std::string& failureClass) {
        DebugRecipe r;
        r.failureClass = failureClass;
        if (failureClass == "compile") {
            r.steps = {
                DebugRecipeStepModel::make(1, "Run single target compile", "deterministic compile error"),
                DebugRecipeStepModel::make(2, "Inspect first compiler diagnostic", "root parse/type issue"),
                DebugRecipeStepModel::make(3, "Patch smallest failing unit", "compile green")
            };
        } else if (failureClass == "test") {
            r.steps = {
                DebugRecipeStepModel::make(1, "Run failing test only", "single deterministic failure"),
                DebugRecipeStepModel::make(2, "Reduce fixture and mocks", "minimal repro"),
                DebugRecipeStepModel::make(3, "Apply constrained patch", "test green")
            };
        } else {
            r.steps = {
                DebugRecipeStepModel::make(1, "Capture failure packet", "classified failure"),
                DebugRecipeStepModel::make(2, "Cluster by root symptom", "primary cluster"),
                DebugRecipeStepModel::make(3, "Apply guarded fix", "regression-safe result")
            };
        }
        return r;
    }

    static nlohmann::json toJson(const DebugRecipe& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& s : r.steps) arr.push_back(DebugRecipeStepModel::toJson(s));
        return {{"failure_class", r.failureClass}, {"steps", arr}};
    }
};
