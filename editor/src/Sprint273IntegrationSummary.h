#pragma once
#include <string>

namespace whetstone {

struct Sprint273IntegrationSummary {
    int  stepsCompleted          = 5;
    int  bindingPairsImplemented = 2;   // RustPython + GoCpp
    bool success                 = true;

    std::string sprintName() const {
        return "Sprint 273: PolyglotFFIGlueGenerator";
    }
};

} // namespace whetstone
