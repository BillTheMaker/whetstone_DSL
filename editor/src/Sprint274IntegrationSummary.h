#pragma once
#include <string>

namespace whetstone {

struct Sprint274IntegrationSummary {
    int  stepsCompleted          = 5;
    int  bindingPairsImplemented = 4;  // RustPython, GoCpp, RustGo, CppJS
    int  mcpToolsAdded           = 1;  // whetstone_generate_ffi_glue
    bool success                 = true;

    std::string sprintName() const {
        return "Sprint 274: More Binding Pairs + DWARF Annotations";
    }
};

} // namespace whetstone
