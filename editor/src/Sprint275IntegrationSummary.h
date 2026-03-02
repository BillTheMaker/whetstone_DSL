#pragma once
#include <string>

namespace whetstone {

struct Sprint275IntegrationSummary {
    int  stepsCompleted = 5;
    int  mcpToolsAdded  = 1;   // whetstone_emit_symbol_index
    bool success        = true;

    std::string sprintName() const {
        return "Sprint 275: Cross-Language Symbol Index";
    }
};

} // namespace whetstone
