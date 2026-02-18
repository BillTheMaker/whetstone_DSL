#pragma once
// Step 632: Agent task status overlay model

#include "AgentTaskSlots.h"

#include <string>
#include <vector>

struct AgentTaskStatusSummary {
    int running = 0;
    int pending = 0;
    std::string label;
    bool hasWork = false;
};

struct AgentTaskOverlayRow {
    std::string taskId;
    std::string description;
    std::string status;
    int elapsedSeconds = 0;
    int toolCallsMade = 0;
    std::string currentStep;
    bool canCancel = false;
    int slotIndex = -1;
};

class AgentTaskStatusOverlay {
public:
    static AgentTaskStatusSummary buildSummary(const AgentTaskSlotsState& state) {
        AgentTaskStatusSummary summary;
        summary.running = AgentTaskSlots::runningCount(state);
        summary.pending = AgentTaskSlots::pendingCount(state);
        summary.hasWork = (summary.running + summary.pending) > 0;
        summary.label = "[Agent: " + std::to_string(summary.running) +
                        " running, " + std::to_string(summary.pending) + " pending]";
        return summary;
    }

    static std::vector<AgentTaskOverlayRow> buildRows(const AgentTaskSlotsState& state,
                                                      int nowSeconds) {
        std::vector<AgentTaskOverlayRow> rows;
        rows.reserve(state.tasks.size());
        for (const auto& task : state.tasks) {
            AgentTaskOverlayRow row;
            row.taskId = task.taskId;
            row.description = task.description;
            row.status = statusText(task.status);
            row.toolCallsMade = task.toolCallsMade;
            row.currentStep = task.currentStep.empty() ? "-" : task.currentStep;
            row.slotIndex = task.slotIndex;
            row.canCancel = task.status == AgentTaskStatus::Pending ||
                            task.status == AgentTaskStatus::Running;
            row.elapsedSeconds = elapsedSeconds(task, nowSeconds);
            rows.push_back(row);
        }
        return rows;
    }

    static bool cancelTask(AgentTaskSlotsState* state, const std::string& taskId) {
        return AgentTaskSlots::cancelTask(state, taskId);
    }

    static std::string statusText(AgentTaskStatus status) {
        if (status == AgentTaskStatus::Pending) return "pending";
        if (status == AgentTaskStatus::Running) return "running";
        if (status == AgentTaskStatus::Completed) return "completed";
        return "cancelled";
    }

private:
    static int elapsedSeconds(const AgentBackgroundTask& task, int nowSeconds) {
        if (task.status == AgentTaskStatus::Running &&
            task.startedAtSeconds > 0 &&
            nowSeconds >= task.startedAtSeconds) {
            return nowSeconds - task.startedAtSeconds;
        }
        return task.lastElapsedSeconds;
    }
};
