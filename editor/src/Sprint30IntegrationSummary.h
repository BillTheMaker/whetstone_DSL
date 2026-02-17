#pragma once
// Step 563: Sprint 30 Integration + Summary

#include <string>
#include <vector>

struct Sprint30Signals {
    bool debugSessionModel = false;          // 554
    bool breakpointSystem = false;           // 555
    bool stepEngine = false;                 // 556
    bool exceptionStackCapture = false;      // 557
    bool phase30aIntegration = false;        // 558
    bool callStackInspector = false;         // 559
    bool localsWatchesModel = false;         // 560
    bool traceTimelineModel = false;         // 561
    bool workflowAwareDebugHooks = false;    // 562
};

struct Sprint30IntegrationResult {
    bool phase30aPass = false;
    bool phase30bPass = false;
    bool sprint30Pass = false;
    std::vector<std::string> notes;
};

class Sprint30IntegrationSummary {
public:
    static Sprint30IntegrationResult summarize(const Sprint30Signals& s) {
        Sprint30IntegrationResult r;
        r.phase30aPass = s.debugSessionModel &&
                         s.breakpointSystem &&
                         s.stepEngine &&
                         s.exceptionStackCapture &&
                         s.phase30aIntegration;

        r.phase30bPass = s.callStackInspector &&
                         s.localsWatchesModel &&
                         s.traceTimelineModel &&
                         s.workflowAwareDebugHooks;

        r.sprint30Pass = r.phase30aPass && r.phase30bPass;
        r.notes = buildNotes(s, r);
        return r;
    }

private:
    static std::vector<std::string> buildNotes(const Sprint30Signals& s,
                                               const Sprint30IntegrationResult& r) {
        std::vector<std::string> notes;
        if (!s.debugSessionModel) notes.push_back("fail:step554_debug_session_model");
        if (!s.breakpointSystem) notes.push_back("fail:step555_breakpoint_system");
        if (!s.stepEngine) notes.push_back("fail:step556_step_engine");
        if (!s.exceptionStackCapture) notes.push_back("fail:step557_exception_capture");
        if (!s.phase30aIntegration) notes.push_back("fail:step558_phase30a_integration");
        if (!s.callStackInspector) notes.push_back("fail:step559_callstack_inspector");
        if (!s.localsWatchesModel) notes.push_back("fail:step560_locals_watches_model");
        if (!s.traceTimelineModel) notes.push_back("fail:step561_trace_timeline_model");
        if (!s.workflowAwareDebugHooks) notes.push_back("fail:step562_workflow_debug_hooks");

        if (r.sprint30Pass) {
            notes.push_back("sprint30:debugger_baseline_daily_use_ready");
            notes.push_back("sprint30:breakpoint_step_exception_cycle_stable");
            notes.push_back("sprint30:runtime_inspection_surfaces_connected");
        } else {
            if (!r.phase30aPass) notes.push_back("phase30a:blocked");
            if (!r.phase30bPass) notes.push_back("phase30b:blocked");
        }
        return notes;
    }
};
