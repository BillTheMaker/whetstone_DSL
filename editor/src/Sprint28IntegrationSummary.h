#pragma once
// Step 543: Sprint 28 Integration + Summary

#include <string>
#include <vector>

struct Sprint28Signals {
    bool operationSelectorApi = false;           // Step 534
    bool symbolSelectorApi = false;              // Step 535
    bool argumentShapeValidator = false;         // Step 536
    bool constrainedExecutionTelemetry = false;  // Step 537
    bool phase28aIntegration = false;            // Step 538
    bool cppAdapter = false;                     // Step 539
    bool pyTsAdapter = false;                    // Step 540
    bool rustGoAdapter = false;                  // Step 541
    bool crossLanguageConsistencyGate = false;   // Step 542
};

struct Sprint28IntegrationResult {
    bool phase28aPass = false;
    bool phase28bPass = false;
    bool sprint28Pass = false;
    std::vector<std::string> notes;
};

class Sprint28IntegrationSummary {
public:
    static Sprint28IntegrationResult summarize(const Sprint28Signals& s) {
        Sprint28IntegrationResult r;
        r.phase28aPass = s.operationSelectorApi &&
                         s.symbolSelectorApi &&
                         s.argumentShapeValidator &&
                         s.constrainedExecutionTelemetry &&
                         s.phase28aIntegration;

        r.phase28bPass = s.cppAdapter &&
                         s.pyTsAdapter &&
                         s.rustGoAdapter &&
                         s.crossLanguageConsistencyGate;

        r.sprint28Pass = r.phase28aPass && r.phase28bPass;
        r.notes = buildNotes(s, r);
        return r;
    }

private:
    static std::vector<std::string> buildNotes(const Sprint28Signals& s,
                                               const Sprint28IntegrationResult& r) {
        std::vector<std::string> notes;
        if (!s.operationSelectorApi) notes.push_back("fail:step534_operation_selector");
        if (!s.symbolSelectorApi) notes.push_back("fail:step535_symbol_selector");
        if (!s.argumentShapeValidator) notes.push_back("fail:step536_argument_shape_validator");
        if (!s.constrainedExecutionTelemetry) notes.push_back("fail:step537_constrained_telemetry");
        if (!s.phase28aIntegration) notes.push_back("fail:step538_phase28a_integration");
        if (!s.cppAdapter) notes.push_back("fail:step539_cpp_adapter");
        if (!s.pyTsAdapter) notes.push_back("fail:step540_pyts_adapter");
        if (!s.rustGoAdapter) notes.push_back("fail:step541_rustgo_adapter");
        if (!s.crossLanguageConsistencyGate) notes.push_back("fail:step542_cross_language_consistency");

        if (r.sprint28Pass) {
            notes.push_back("sprint28:legal_choice_execution_stable");
            notes.push_back("sprint28:multi_language_adapters_consistent");
            notes.push_back("sprint28:out_of_scope_and_hallucinated_edits_blocked");
        } else {
            if (!r.phase28aPass) notes.push_back("phase28a:blocked");
            if (!r.phase28bPass) notes.push_back("phase28b:blocked");
        }
        return notes;
    }
};
