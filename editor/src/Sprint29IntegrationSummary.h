#pragma once
// Step 553: Sprint 29 Integration + Summary

#include <string>
#include <vector>

struct Sprint29Signals {
    bool constrainedRoutingRuleset = false;   // 544
    bool contextBundleMinimizer = false;      // 545
    bool confidenceCalibrator = false;        // 546
    bool costPolicyGuard = false;             // 547
    bool phase29aIntegration = false;         // 548
    bool costSuggestionEngine = false;        // 549
    bool workerEfficiencyDashboard = false;   // 550
    bool reviewGateRefinement = false;        // 551
    bool costQualityRegressionSuite = false;  // 552
};

struct Sprint29IntegrationResult {
    bool phase29aPass = false;
    bool phase29bPass = false;
    bool sprint29Pass = false;
    std::vector<std::string> notes;
};

class Sprint29IntegrationSummary {
public:
    static Sprint29IntegrationResult summarize(const Sprint29Signals& s) {
        Sprint29IntegrationResult r;
        r.phase29aPass = s.constrainedRoutingRuleset &&
                         s.contextBundleMinimizer &&
                         s.confidenceCalibrator &&
                         s.costPolicyGuard &&
                         s.phase29aIntegration;

        r.phase29bPass = s.costSuggestionEngine &&
                         s.workerEfficiencyDashboard &&
                         s.reviewGateRefinement &&
                         s.costQualityRegressionSuite;

        r.sprint29Pass = r.phase29aPass && r.phase29bPass;
        r.notes = buildNotes(s, r);
        return r;
    }

private:
    static std::vector<std::string> buildNotes(const Sprint29Signals& s,
                                               const Sprint29IntegrationResult& r) {
        std::vector<std::string> notes;
        if (!s.constrainedRoutingRuleset) notes.push_back("fail:step544_routing_ruleset");
        if (!s.contextBundleMinimizer) notes.push_back("fail:step545_context_minimizer");
        if (!s.confidenceCalibrator) notes.push_back("fail:step546_confidence_calibrator");
        if (!s.costPolicyGuard) notes.push_back("fail:step547_cost_policy_guard");
        if (!s.phase29aIntegration) notes.push_back("fail:step548_phase29a_integration");
        if (!s.costSuggestionEngine) notes.push_back("fail:step549_cost_suggestions");
        if (!s.workerEfficiencyDashboard) notes.push_back("fail:step550_worker_dashboard");
        if (!s.reviewGateRefinement) notes.push_back("fail:step551_review_gate_refinement");
        if (!s.costQualityRegressionSuite) notes.push_back("fail:step552_cost_quality_regression");

        if (r.sprint29Pass) {
            notes.push_back("sprint29:routing_and_cost_policy_coherent");
            notes.push_back("sprint29:context_width_reduced_without_quality_regression");
            notes.push_back("sprint29:cost_controls_active_with_safety_preserved");
        } else {
            if (!r.phase29aPass) notes.push_back("phase29a:blocked");
            if (!r.phase29bPass) notes.push_back("phase29b:blocked");
        }
        return notes;
    }
};
