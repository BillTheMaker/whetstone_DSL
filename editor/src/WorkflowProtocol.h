#pragma once
// Step 384: Workflow session protocol for MCP-driven orchestration.

#include "WorkflowState.h"
#include <algorithm>
#include <string>
#include <vector>

struct WorkflowSession {
    std::string sessionId;
    std::string projectName;
    std::string currentPhase = "init";
    std::string startedAt = workItemTimestamp();
    std::vector<std::string> commands;

    json toJson() const {
        return {
            {"sessionId", sessionId},
            {"projectName", projectName},
            {"currentPhase", currentPhase},
            {"startedAt", startedAt},
            {"commands", commands}
        };
    }

    static WorkflowSession fromJson(const json& j) {
        WorkflowSession s;
        s.sessionId = j.value("sessionId", "");
        s.projectName = j.value("projectName", "");
        s.currentPhase = j.value("currentPhase", "init");
        s.startedAt = j.value("startedAt", workItemTimestamp());
        if (j.contains("commands") && j["commands"].is_array()) {
            s.commands = j["commands"].get<std::vector<std::string>>();
        }
        return s;
    }
};

struct ProtocolAction {
    std::string phase;
    std::string command;
    std::string reason;

    json toJson() const {
        return {{"phase", phase}, {"command", command}, {"reason", reason}};
    }
};

inline std::vector<std::string> workflowProtocolPhases() {
    return {"init", "model", "annotate", "plan", "route",
            "execute", "assist", "review", "complete"};
}

inline bool isWorkflowProtocolPhase(const std::string& phase) {
    const auto phases = workflowProtocolPhases();
    return std::find(phases.begin(), phases.end(), phase) != phases.end();
}

inline int workflowPhaseIndex(const std::string& phase) {
    const auto phases = workflowProtocolPhases();
    for (size_t i = 0; i < phases.size(); ++i) {
        if (phases[i] == phase) return static_cast<int>(i);
    }
    return -1;
}

inline bool validateTransition(const std::string& from, const std::string& to) {
    int fromIdx = workflowPhaseIndex(from);
    int toIdx = workflowPhaseIndex(to);
    if (fromIdx < 0 || toIdx < 0) return false;
    if (fromIdx == toIdx) return true;
    return toIdx == fromIdx + 1;
}

inline std::string protocolPhaseForCommand(const std::string& command) {
    if (command == "whetstone_create_skeleton") return "model";
    if (command == "whetstone_add_skeleton_node") return "model";
    if (command == "whetstone_infer_annotations") return "annotate";
    if (command == "whetstone_create_workflow") return "plan";
    if (command == "whetstone_orchestrate_advance") return "route";
    if (command == "whetstone_orchestrate_run_deterministic") return "execute";
    if (command == "whetstone_get_blockers" || command == "whetstone_submit_result")
        return "assist";
    if (command == "whetstone_get_review_policy" || command == "whetstone_set_review_policy")
        return "review";
    if (command == "whetstone_save_workflow") return "complete";
    return "";
}

inline void recordWorkflowCommand(WorkflowSession& session,
                                  const std::string& command) {
    session.commands.push_back(command);
    std::string hintedPhase = protocolPhaseForCommand(command);
    if (hintedPhase.empty()) return;
    int current = workflowPhaseIndex(session.currentPhase);
    int hinted = workflowPhaseIndex(hintedPhase);
    if (hinted > current) session.currentPhase = hintedPhase;
}

inline std::string detectProtocolPhaseFromWorkflow(const WorkflowState& workflow) {
    auto stats = workflow.getStats();
    if (stats.total == 0) return "plan";
    if (stats.complete == stats.total) return "complete";
    if (stats.review > 0) return "review";

    bool hasExternalOrHuman = false;
    for (const auto& status : {WI_ASSIGNED, WI_IN_PROGRESS}) {
        for (const auto& item : workflow.queue.getByStatus(status)) {
            if (item.workerType == "slm" || item.workerType == "llm" ||
                item.workerType == "human") {
                hasExternalOrHuman = true;
            }
        }
    }
    if (hasExternalOrHuman) return "assist";
    if (stats.ready > 0 || stats.assigned > 0 || stats.inProgress > 0) {
        return "execute";
    }
    return "route";
}

inline ProtocolAction getNextAction(const WorkflowSession& session,
                                    const WorkflowState* workflow = nullptr) {
    std::string phase = session.currentPhase;
    if (workflow) {
        std::string inferred = detectProtocolPhaseFromWorkflow(*workflow);
        if (workflowPhaseIndex(inferred) > workflowPhaseIndex(phase)) {
            phase = inferred;
        }
    }

    if (phase == "init") {
        return {"init", "whetstone_create_skeleton", "Initialize project skeleton."};
    }
    if (phase == "model") {
        return {"model", "whetstone_add_skeleton_node",
                "Add skeleton functions/classes until model is complete."};
    }
    if (phase == "annotate") {
        return {"annotate", "whetstone_infer_annotations",
                "Infer and adjust routing annotations before planning."};
    }
    if (phase == "plan") {
        return {"plan", "whetstone_create_workflow",
                "Create workflow tasks from skeleton annotations."};
    }
    if (phase == "route") {
        return {"route", "whetstone_orchestrate_advance",
                "Route and prepare ready tasks in a single advance batch."};
    }
    if (phase == "execute") {
        return {"execute", "whetstone_orchestrate_run_deterministic",
                "Complete deterministic/template tasks automatically."};
    }
    if (phase == "assist") {
        return {"assist", "whetstone_get_blockers",
                "Fetch blockers, then submit external results as needed."};
    }
    if (phase == "review") {
        return {"review", "whetstone_get_workflow_state",
                "Resolve review items and re-run deterministic orchestration."};
    }
    return {"complete", "whetstone_save_workflow",
            "Persist completed workflow state."};
}

inline json getSessionSummary(const WorkflowSession& session,
                              const WorkflowState* workflow = nullptr) {
    ProtocolAction next = getNextAction(session, workflow);
    json summary = {
        {"sessionId", session.sessionId},
        {"projectName", session.projectName},
        {"currentPhase", session.currentPhase},
        {"startedAt", session.startedAt},
        {"commandCount", (int)session.commands.size()},
        {"commands", session.commands},
        {"nextAction", next.toJson()}
    };
    if (workflow) {
        summary["workflowPhase"] = workflowPhaseToString(workflow->getPhase());
        summary["workflowStats"] = workflow->getStats().toJson();
        summary["inferredPhase"] = detectProtocolPhaseFromWorkflow(*workflow);
    }
    return summary;
}
