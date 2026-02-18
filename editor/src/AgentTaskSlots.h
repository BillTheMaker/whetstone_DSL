#pragma once
// Step 631: Background task slots

#include <string>
#include <vector>

enum class AgentTaskStatus {
    Pending,
    Running,
    Completed,
    Cancelled
};

struct AgentBackgroundTask {
    std::string taskId;
    std::string description;
    AgentTaskStatus status = AgentTaskStatus::Pending;
    int slotIndex = -1;
};

struct AgentTaskSlotsState {
    int maxSlots = 2;
    int nextTaskId = 1;
    std::vector<AgentBackgroundTask> tasks;
};

class AgentTaskSlots {
public:
    static bool setMaxSlots(AgentTaskSlotsState* state, int maxSlots) {
        if (!state || maxSlots < 1) return false;
        state->maxSlots = maxSlots;
        return true;
    }

    static std::string enqueueTask(AgentTaskSlotsState* state, const std::string& description) {
        if (!state || description.empty()) return "";
        AgentBackgroundTask task;
        task.taskId = "task-slot-" + std::to_string(state->nextTaskId++);
        task.description = description;
        task.status = AgentTaskStatus::Pending;
        state->tasks.push_back(task);
        return task.taskId;
    }

    static int assignPendingToSlots(AgentTaskSlotsState* state) {
        if (!state) return 0;
        int assigned = 0;
        int running = runningCount(*state);
        for (auto& task : state->tasks) {
            if (running >= state->maxSlots) break;
            if (task.status != AgentTaskStatus::Pending) continue;
            task.status = AgentTaskStatus::Running;
            task.slotIndex = nextAvailableSlot(*state);
            ++running;
            ++assigned;
        }
        return assigned;
    }

    static bool completeTask(AgentTaskSlotsState* state, const std::string& taskId) {
        AgentBackgroundTask* task = findMutable(state, taskId);
        if (!task || task->status != AgentTaskStatus::Running) return false;
        task->status = AgentTaskStatus::Completed;
        task->slotIndex = -1;
        return true;
    }

    static bool cancelTask(AgentTaskSlotsState* state, const std::string& taskId) {
        AgentBackgroundTask* task = findMutable(state, taskId);
        if (!task) return false;
        task->status = AgentTaskStatus::Cancelled;
        task->slotIndex = -1;
        return true;
    }

    static int runningCount(const AgentTaskSlotsState& state) {
        return countByStatus(state, AgentTaskStatus::Running);
    }

    static int pendingCount(const AgentTaskSlotsState& state) {
        return countByStatus(state, AgentTaskStatus::Pending);
    }

    static int completedCount(const AgentTaskSlotsState& state) {
        return countByStatus(state, AgentTaskStatus::Completed);
    }

private:
    static int countByStatus(const AgentTaskSlotsState& state, AgentTaskStatus status) {
        int count = 0;
        for (const auto& task : state.tasks) {
            if (task.status == status) ++count;
        }
        return count;
    }

    static int nextAvailableSlot(const AgentTaskSlotsState& state) {
        for (int slot = 0; slot < state.maxSlots; ++slot) {
            bool used = false;
            for (const auto& task : state.tasks) {
                if (task.status == AgentTaskStatus::Running &&
                    task.slotIndex == slot) {
                    used = true;
                    break;
                }
            }
            if (!used) return slot;
        }
        return 0;
    }

    static AgentBackgroundTask* findMutable(AgentTaskSlotsState* state,
                                            const std::string& taskId) {
        if (!state) return nullptr;
        for (auto& task : state->tasks) {
            if (task.taskId == taskId) return &task;
        }
        return nullptr;
    }
};
