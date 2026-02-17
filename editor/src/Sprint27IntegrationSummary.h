#pragma once
// Step 533: Sprint 27 Integration + Summary

#include <string>
#include <vector>

struct Sprint27Signals {
    bool taskitemContractSchema = false;      // Step 524
    bool legalOperationGraph = false;         // Step 525
    bool symbolScopeExtractor = false;        // Step 526
    bool constraintDiagnostics = false;       // Step 527
    bool phase27aIntegrationGate = false;     // Step 528
    bool preApplyValidationGate = false;      // Step 529
    bool postApplyStructuralGate = false;     // Step 530
    bool contractDeltaChecker = false;        // Step 531
    bool retryEscalationProtocol = false;     // Step 532
};

struct Sprint27IntegrationResult {
    bool phase27aPass = false;
    bool phase27bPass = false;
    bool sprint27Pass = false;
    std::vector<std::string> notes;
};

class Sprint27IntegrationSummary {
public:
    static Sprint27IntegrationResult summarize(const Sprint27Signals& s) {
        Sprint27IntegrationResult r;

        r.phase27aPass = s.taskitemContractSchema &&
                         s.legalOperationGraph &&
                         s.symbolScopeExtractor &&
                         s.constraintDiagnostics &&
                         s.phase27aIntegrationGate;

        r.phase27bPass = s.preApplyValidationGate &&
                         s.postApplyStructuralGate &&
                         s.contractDeltaChecker &&
                         s.retryEscalationProtocol;

        r.sprint27Pass = r.phase27aPass && r.phase27bPass;
        r.notes = buildNotes(s, r);
        return r;
    }

private:
    static std::vector<std::string> buildNotes(const Sprint27Signals& s,
                                               const Sprint27IntegrationResult& r) {
        std::vector<std::string> notes;
        if (!s.taskitemContractSchema) notes.push_back("fail:step524_taskitem_contract_schema");
        if (!s.legalOperationGraph) notes.push_back("fail:step525_legal_operation_graph");
        if (!s.symbolScopeExtractor) notes.push_back("fail:step526_symbol_scope_extractor");
        if (!s.constraintDiagnostics) notes.push_back("fail:step527_constraint_diagnostics");
        if (!s.phase27aIntegrationGate) notes.push_back("fail:step528_phase27a_integration");
        if (!s.preApplyValidationGate) notes.push_back("fail:step529_pre_apply_validation");
        if (!s.postApplyStructuralGate) notes.push_back("fail:step530_post_apply_structural_gate");
        if (!s.contractDeltaChecker) notes.push_back("fail:step531_contract_delta_checker");
        if (!s.retryEscalationProtocol) notes.push_back("fail:step532_retry_escalation_protocol");

        if (r.sprint27Pass) {
            notes.push_back("sprint27:constrained_pipeline_pass");
            notes.push_back("sprint27:out_of_scope_edits_prevented_by_construction");
            notes.push_back("sprint27:hallucinated_symbol_calls_fail_fast");
        } else {
            if (!r.phase27aPass) notes.push_back("phase27a:blocked");
            if (!r.phase27bPass) notes.push_back("phase27b:blocked");
        }
        return notes;
    }
};
