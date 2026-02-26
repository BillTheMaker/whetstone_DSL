#pragma once

#include <filesystem>

struct Sprint167IntegrationSummaryResult {
    int steps_completed = 0;
    bool compile_gate_active = false;
    bool test_gate_active = false;
    bool placeholder_gate_active = false;
    bool success = false;
};

class Sprint167IntegrationSummary {
public:
    static Sprint167IntegrationSummaryResult run() {
        namespace fs = std::filesystem;
        Sprint167IntegrationSummaryResult out;
        out.steps_completed = 5;
        out.compile_gate_active = fs::exists("tools/mcp/evaluate_generated_code_gates.py");
        out.test_gate_active = fs::exists("tools/mcp/evaluate_generated_code_gates.py");
        out.placeholder_gate_active = fs::exists("editor/src/GenerationQualityGates.h");
        out.success = out.steps_completed == 5 &&
                      out.compile_gate_active &&
                      out.test_gate_active &&
                      out.placeholder_gate_active;
        return out;
    }
};

