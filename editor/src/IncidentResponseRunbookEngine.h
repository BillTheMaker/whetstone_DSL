#pragma once
// Step 599: Incident Response Runbook Engine

#include <map>
#include <string>
#include <vector>

struct RunbookStep {
    std::string stepId;
    std::string instruction;
    bool required = true;
};

struct Runbook {
    std::string runbookId;
    std::string incidentType;
    std::vector<RunbookStep> steps;
};

struct RunbookExecutionState {
    std::string runbookId;
    std::vector<std::string> completedStepIds;
    bool closed = false;
};

class IncidentResponseRunbookEngine {
public:
    bool registerRunbook(const Runbook& runbook, std::string* error) {
        if (!error) return false;
        error->clear();
        if (runbook.runbookId.empty()) return fail(error, "runbook_id_missing");
        if (runbook.incidentType.empty()) return fail(error, "incident_type_missing");
        if (runbook.steps.empty()) return fail(error, "runbook_steps_missing");
        if (runbooks_.count(runbook.runbookId) != 0) return fail(error, "runbook_duplicate");
        runbooks_[runbook.runbookId] = runbook;
        return true;
    }

    bool start(const std::string& runbookId, RunbookExecutionState* state, std::string* error) const {
        if (!state || !error) return false;
        error->clear();
        if (runbooks_.count(runbookId) == 0) return fail(error, "runbook_missing");
        state->runbookId = runbookId;
        state->completedStepIds.clear();
        state->closed = false;
        return true;
    }

    bool completeStep(RunbookExecutionState* state,
                      const std::string& stepId,
                      std::string* error) const {
        if (!state || !error) return false;
        error->clear();
        const auto* rb = findRunbook(state->runbookId);
        if (!rb) return fail(error, "runbook_missing");
        if (state->closed) return fail(error, "execution_closed");
        if (!hasStep(*rb, stepId)) return fail(error, "step_missing");
        for (const auto& s : state->completedStepIds) if (s == stepId) return fail(error, "step_already_completed");
        state->completedStepIds.push_back(stepId);
        return true;
    }

    bool closeIfReady(RunbookExecutionState* state, std::string* error) const {
        if (!state || !error) return false;
        error->clear();
        const auto* rb = findRunbook(state->runbookId);
        if (!rb) return fail(error, "runbook_missing");
        for (const auto& step : rb->steps) {
            if (!step.required) continue;
            if (!isCompleted(*state, step.stepId)) return fail(error, "required_steps_incomplete");
        }
        state->closed = true;
        return true;
    }

    int completionPercent(const RunbookExecutionState& state) const {
        const auto* rb = findRunbook(state.runbookId);
        if (!rb || rb->steps.empty()) return 0;
        const int done = static_cast<int>(state.completedStepIds.size());
        return (done * 100) / static_cast<int>(rb->steps.size());
    }

private:
    std::map<std::string, Runbook> runbooks_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    const Runbook* findRunbook(const std::string& runbookId) const {
        auto it = runbooks_.find(runbookId);
        if (it == runbooks_.end()) return nullptr;
        return &it->second;
    }

    static bool hasStep(const Runbook& runbook, const std::string& stepId) {
        for (const auto& s : runbook.steps) if (s.stepId == stepId) return true;
        return false;
    }

    static bool isCompleted(const RunbookExecutionState& state, const std::string& stepId) {
        for (const auto& s : state.completedStepIds) if (s == stepId) return true;
        return false;
    }
};
