#pragma once
// Step 558: Phase 30a Integration

#include <string>
#include <vector>

#include "DebugSessionModel.h"
#include "BreakpointSystem.h"
#include "StepEngine.h"
#include "ExceptionStackTraceCapture.h"

struct Phase30aRequest {
    std::string targetId;
    std::string bufferId;
    std::string executablePath;
    std::string breakpointId;
    int breakpointLine = 0;
    bool triggerCondition = true;
    std::string stepCommand;
    bool triggerException = false;
};

struct Phase30aResult {
    bool pass = false;
    bool breakpointHit = false;
    StepResult stepResult;
    ExceptionCaptureResult exceptionResult;
    std::vector<std::string> errors;
};

class Phase30aIntegration {
public:
    static Phase30aResult run(const Phase30aRequest& request) {
        Phase30aResult out;

        DebugSessionModel session;
        if (!session.start(request.targetId, request.bufferId, request.executablePath)) {
            out.errors.push_back("session_start_failed");
            return out;
        }

        BreakpointSystem breakpoints;
        if (!breakpoints.addConditionalBreakpoint(request.breakpointId,
                                                  request.bufferId,
                                                  request.breakpointLine,
                                                  "runtime_condition")) {
            out.errors.push_back("breakpoint_add_failed");
            return out;
        }

        out.breakpointHit = breakpoints.shouldBreak(request.breakpointId, request.triggerCondition);
        if (!out.breakpointHit) {
            out.errors.push_back("breakpoint_not_hit");
            return out;
        }

        StepEngine engine;
        out.stepResult = engine.execute(request.stepCommand);
        if (!out.stepResult.ok) {
            out.errors.push_back("step_failed");
            return out;
        }

        if (request.triggerException) {
            out.exceptionResult = ExceptionStackTraceCapture::capture(
                "RuntimeError",
                "simulated exception",
                "thread-main",
                {{"run", request.bufferId, request.breakpointLine, ""},
                 {"dispatch", request.bufferId, request.breakpointLine + 1, ""}});
            if (!out.exceptionResult.valid) {
                out.errors.push_back("exception_capture_failed");
                return out;
            }
        }

        out.pass = true;
        return out;
    }
};
