#pragma once
// Step 1528: Sprint 128 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "debug/DebugRecipeStep.h"
#include "debug/DebugRecipeLibrary.h"
#include "debug/DebugActionValidator.h"
#include "debug/DebugExecutionTrace.h"
#include "debug/DebugRecipePacket.h"

struct Sprint128IntegrationResult {
    bool stepReady = false;
    bool recipeReady = false;
    bool validatorReady = false;
    bool traceReady = false;
    bool packetReady = false;
    bool recipeToolReady = false;
    bool validateToolReady = false;
    bool recordTraceToolReady = false;
    bool getTraceToolReady = false;
    bool success = false;
    int stepStart = 1519;
    int stepEnd = 1528;
    std::vector<std::string> filesAdded;
};

class Sprint128IntegrationSummary {
public:
    static Sprint128IntegrationResult run() {
        Sprint128IntegrationResult out;
        out.filesAdded = {
            "debug/DebugRecipeStep.h",
            "debug/DebugRecipeLibrary.h",
            "debug/DebugActionValidator.h",
            "debug/DebugExecutionTrace.h",
            "debug/DebugRecipePacket.h",
            "mcp/RegisterDebugWorkflowTools.h",
            "Sprint128IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto step = DebugRecipeStepModel::make(1, "a", "e");
        out.stepReady = step.index == 1;

        auto recipe = DebugRecipeLibrary::get("compile");
        out.recipeReady = !recipe.steps.empty();

        auto v = DebugActionValidator::validate("edit", false, false, false);
        out.validatorReady = v.allowed;

        auto trace = DebugExecutionTrace::append({}, {1, "act", "ok"});
        out.traceReady = !trace.empty();

        out.packetReady = DebugRecipePacketModel::toJson(DebugRecipePacketModel::build(recipe, v, trace)).contains("recipe");

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            std::string n = t.value("name", "");
            if (n == "whetstone_get_debug_recipe") out.recipeToolReady = true;
            if (n == "whetstone_validate_debug_action") out.validateToolReady = true;
            if (n == "whetstone_record_debug_trace") out.recordTraceToolReady = true;
            if (n == "whetstone_get_debug_trace") out.getTraceToolReady = true;
        }

        out.success = out.stepReady && out.recipeReady && out.validatorReady && out.traceReady && out.packetReady &&
                      out.recipeToolReady && out.validateToolReady && out.recordTraceToolReady && out.getTraceToolReady &&
                      out.stepStart == 1519 && out.stepEnd == 1528;
        return out;
    }
};
