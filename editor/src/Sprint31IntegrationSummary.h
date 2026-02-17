#pragma once
// Step 573: Sprint 31 Integration + Summary

#include <string>
#include <vector>

struct Sprint31Signals {
    bool memorySnapshotModel = false;            // 564
    bool memoryInspectorUiModel = false;         // 565
    bool allocationOwnershipTraceHooks = false;  // 566
    bool leakCorruptionSignalBridge = false;     // 567
    bool phase31aIntegration = false;            // 568
    bool disassemblyRegisterView = false;        // 569
    bool watchEvaluatorHardening = false;        // 570
    bool timeTravelEventBuffer = false;          // 571
    bool performanceProbeOverlay = false;        // 572
};

struct Sprint31IntegrationResult {
    bool phase31aPass = false;
    bool phase31bPass = false;
    bool sprint31Pass = false;
    std::vector<std::string> notes;
};

class Sprint31IntegrationSummary {
public:
    static Sprint31IntegrationResult summarize(const Sprint31Signals& s) {
        Sprint31IntegrationResult r;
        r.phase31aPass = s.memorySnapshotModel &&
                         s.memoryInspectorUiModel &&
                         s.allocationOwnershipTraceHooks &&
                         s.leakCorruptionSignalBridge &&
                         s.phase31aIntegration;

        r.phase31bPass = s.disassemblyRegisterView &&
                         s.watchEvaluatorHardening &&
                         s.timeTravelEventBuffer &&
                         s.performanceProbeOverlay;

        r.sprint31Pass = r.phase31aPass && r.phase31bPass;
        r.notes = buildNotes(s, r);
        return r;
    }

private:
    static std::vector<std::string> buildNotes(const Sprint31Signals& s,
                                               const Sprint31IntegrationResult& r) {
        std::vector<std::string> notes;
        if (!s.memorySnapshotModel) notes.push_back("fail:step564_memory_snapshot_model");
        if (!s.memoryInspectorUiModel) notes.push_back("fail:step565_memory_inspector_ui_model");
        if (!s.allocationOwnershipTraceHooks) notes.push_back("fail:step566_allocation_ownership_trace_hooks");
        if (!s.leakCorruptionSignalBridge) notes.push_back("fail:step567_leak_corruption_signal_bridge");
        if (!s.phase31aIntegration) notes.push_back("fail:step568_phase31a_integration");
        if (!s.disassemblyRegisterView) notes.push_back("fail:step569_disassembly_register_view");
        if (!s.watchEvaluatorHardening) notes.push_back("fail:step570_watch_evaluator_hardening");
        if (!s.timeTravelEventBuffer) notes.push_back("fail:step571_time_travel_event_buffer");
        if (!s.performanceProbeOverlay) notes.push_back("fail:step572_performance_probe_overlay");

        if (r.sprint31Pass) {
            notes.push_back("sprint31:memory_reflection_workflow_ready");
            notes.push_back("sprint31:advanced_debug_views_stable");
            notes.push_back("sprint31:observability_surfaces_integrated");
        } else {
            if (!r.phase31aPass) notes.push_back("phase31a:blocked");
            if (!r.phase31bPass) notes.push_back("phase31b:blocked");
        }
        return notes;
    }
};
