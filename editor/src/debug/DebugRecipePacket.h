#pragma once
// Step 1527: debug recipe packet model.

#include <nlohmann/json.hpp>

#include "DebugRecipeLibrary.h"
#include "DebugActionValidator.h"
#include "DebugExecutionTrace.h"

struct DebugRecipePacket {
    DebugRecipe recipe;
    DebugActionValidation validation;
    std::vector<DebugTraceEvent> trace;
};

class DebugRecipePacketModel {
public:
    static DebugRecipePacket build(const DebugRecipe& recipe,
                                   const DebugActionValidation& validation,
                                   const std::vector<DebugTraceEvent>& trace) {
        return {recipe, validation, trace};
    }

    static nlohmann::json toJson(const DebugRecipePacket& p) {
        return {
            {"recipe", DebugRecipeLibrary::toJson(p.recipe)},
            {"validation", DebugActionValidator::toJson(p.validation)},
            {"trace", DebugExecutionTrace::toJson(p.trace)}
        };
    }
};
