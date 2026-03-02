#pragma once
#include <string>

namespace whetstone {

struct Sprint276IntegrationSummary {
    int  stepsCompleted = 5;
    bool success        = true;

    std::string sprintName() const {
        return "Sprint 276: LSP Proxy Core";
    }
};

} // namespace whetstone
