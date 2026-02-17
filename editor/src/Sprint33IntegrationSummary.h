#pragma once
// Step 593: Sprint 33 Integration + Program Summary

#include <string>
#include <vector>

struct Sprint33Signals {
    bool onboardingFlow = false;            // 584
    bool workflowVisualization = false;     // 585
    bool capabilityDiscovery = false;       // 586
    bool guidedDemoMode = false;            // 587
    bool phase33aIntegration = false;       // 588
    bool releaseReadinessPack = false;      // 589
    bool crashRecoverySweep = false;        // 590
    bool benchmarkHarness = false;          // 591
    bool docsPlaybooks = false;             // 592
};

struct Sprint33IntegrationResult {
    bool phase33aPass = false;
    bool phase33bPass = false;
    bool sprint33Pass = false;
    std::vector<std::string> notes;
};

class Sprint33IntegrationSummary {
public:
    static Sprint33IntegrationResult summarize(const Sprint33Signals& s) {
        Sprint33IntegrationResult r;
        r.phase33aPass = s.onboardingFlow &&
                         s.workflowVisualization &&
                         s.capabilityDiscovery &&
                         s.guidedDemoMode &&
                         s.phase33aIntegration;

        r.phase33bPass = s.releaseReadinessPack &&
                         s.crashRecoverySweep &&
                         s.benchmarkHarness &&
                         s.docsPlaybooks;

        r.sprint33Pass = r.phase33aPass && r.phase33bPass;
        r.notes = buildNotes(s, r);
        return r;
    }

private:
    static std::vector<std::string> buildNotes(const Sprint33Signals& s,
                                               const Sprint33IntegrationResult& r) {
        std::vector<std::string> notes;
        if (!s.onboardingFlow) notes.push_back("fail:step584_onboarding_flow");
        if (!s.workflowVisualization) notes.push_back("fail:step585_workflow_visualization");
        if (!s.capabilityDiscovery) notes.push_back("fail:step586_capability_discovery");
        if (!s.guidedDemoMode) notes.push_back("fail:step587_guided_demo_mode");
        if (!s.phase33aIntegration) notes.push_back("fail:step588_phase33a_integration");
        if (!s.releaseReadinessPack) notes.push_back("fail:step589_release_readiness_pack");
        if (!s.crashRecoverySweep) notes.push_back("fail:step590_crash_recovery_sweep");
        if (!s.benchmarkHarness) notes.push_back("fail:step591_benchmark_harness");
        if (!s.docsPlaybooks) notes.push_back("fail:step592_docs_playbooks");

        if (r.sprint33Pass) {
            notes.push_back("sprint33:product_experience_clear");
            notes.push_back("sprint33:operational_hardening_ready");
            notes.push_back("sprint33:competitive_readiness_established");
        } else {
            if (!r.phase33aPass) notes.push_back("phase33a:blocked");
            if (!r.phase33bPass) notes.push_back("phase33b:blocked");
        }
        return notes;
    }
};
