#pragma once
// Step 583: Sprint 32 Integration + Summary

#include <string>
#include <vector>

struct Sprint32Signals {
    bool markdownSpecParser = false;             // 574
    bool requirementNormalization = false;       // 575
    bool scopeMilestoneDecomposer = false;       // 576
    bool architectReviewSurface = false;         // 577
    bool phase32aIntegration = false;            // 578
    bool taskitemGeneratorV2 = false;            // 579
    bool confidenceAmbiguityAnnotation = false;  // 580
    bool acceptanceCriteriaBinding = false;      // 581
    bool intakeQueueSimulationHarness = false;   // 582
};

struct Sprint32IntegrationResult {
    bool phase32aPass = false;
    bool phase32bPass = false;
    bool sprint32Pass = false;
    std::vector<std::string> notes;
};

class Sprint32IntegrationSummary {
public:
    static Sprint32IntegrationResult summarize(const Sprint32Signals& s) {
        Sprint32IntegrationResult r;
        r.phase32aPass = s.markdownSpecParser &&
                         s.requirementNormalization &&
                         s.scopeMilestoneDecomposer &&
                         s.architectReviewSurface &&
                         s.phase32aIntegration;

        r.phase32bPass = s.taskitemGeneratorV2 &&
                         s.confidenceAmbiguityAnnotation &&
                         s.acceptanceCriteriaBinding &&
                         s.intakeQueueSimulationHarness;

        r.sprint32Pass = r.phase32aPass && r.phase32bPass;
        r.notes = buildNotes(s, r);
        return r;
    }

private:
    static std::vector<std::string> buildNotes(const Sprint32Signals& s,
                                               const Sprint32IntegrationResult& r) {
        std::vector<std::string> notes;
        if (!s.markdownSpecParser) notes.push_back("fail:step574_markdown_spec_parser");
        if (!s.requirementNormalization) notes.push_back("fail:step575_requirement_normalization");
        if (!s.scopeMilestoneDecomposer) notes.push_back("fail:step576_scope_milestone_decomposer");
        if (!s.architectReviewSurface) notes.push_back("fail:step577_architect_review_surface");
        if (!s.phase32aIntegration) notes.push_back("fail:step578_phase32a_integration");
        if (!s.taskitemGeneratorV2) notes.push_back("fail:step579_taskitem_generator_v2");
        if (!s.confidenceAmbiguityAnnotation) notes.push_back("fail:step580_confidence_ambiguity_annotation");
        if (!s.acceptanceCriteriaBinding) notes.push_back("fail:step581_acceptance_criteria_binding");
        if (!s.intakeQueueSimulationHarness) notes.push_back("fail:step582_intake_queue_simulation_harness");

        if (r.sprint32Pass) {
            notes.push_back("sprint32:architect_intake_pipeline_ready");
            notes.push_back("sprint32:taskitem_queue_quality_signals_ready");
            notes.push_back("sprint32:markdown_to_constrained_execution_stable");
        } else {
            if (!r.phase32aPass) notes.push_back("phase32a:blocked");
            if (!r.phase32bPass) notes.push_back("phase32b:blocked");
        }
        return notes;
    }
};
