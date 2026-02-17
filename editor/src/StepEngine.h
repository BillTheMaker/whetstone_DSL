#pragma once
// Step 556: Step Engine

#include <string>
#include <vector>

enum class DebugRunState {
    Running,
    Paused,
    Completed
};

struct StepEngineState {
    DebugRunState runState = DebugRunState::Paused;
    int stackDepth = 1;
    int instructionPointer = 0;
    std::string lastCommand;
};

struct StepResult {
    bool ok = false;
    StepEngineState state;
    std::vector<std::string> diagnostics;
};

class StepEngine {
public:
    StepEngine() = default;

    const StepEngineState& state() const { return state_; }

    StepResult execute(const std::string& command) {
        StepResult out;
        out.state = state_;

        if (state_.runState == DebugRunState::Completed) {
            out.ok = false;
            out.diagnostics.push_back("session_completed");
            return out;
        }

        if (command == "continue") {
            return onContinue();
        }
        if (command == "step_over") {
            return onStepOver();
        }
        if (command == "step_into") {
            return onStepInto();
        }
        if (command == "step_out") {
            return onStepOut();
        }

        out.ok = false;
        out.diagnostics.push_back("unknown_command");
        return out;
    }

    bool pause() {
        if (state_.runState == DebugRunState::Completed) return false;
        state_.runState = DebugRunState::Paused;
        return true;
    }

private:
    StepEngineState state_;

    StepResult onContinue() {
        StepResult out;
        state_.runState = DebugRunState::Running;
        state_.instructionPointer += 5;
        state_.lastCommand = "continue";
        // Deterministic stop point after "running" burst.
        state_.runState = DebugRunState::Paused;
        out.ok = true;
        out.state = state_;
        return out;
    }

    StepResult onStepOver() {
        StepResult out;
        if (state_.runState != DebugRunState::Paused) {
            out.ok = false;
            out.diagnostics.push_back("must_be_paused");
            out.state = state_;
            return out;
        }
        state_.instructionPointer += 1;
        state_.lastCommand = "step_over";
        out.ok = true;
        out.state = state_;
        return out;
    }

    StepResult onStepInto() {
        StepResult out;
        if (state_.runState != DebugRunState::Paused) {
            out.ok = false;
            out.diagnostics.push_back("must_be_paused");
            out.state = state_;
            return out;
        }
        state_.instructionPointer += 1;
        state_.stackDepth += 1;
        state_.lastCommand = "step_into";
        out.ok = true;
        out.state = state_;
        return out;
    }

    StepResult onStepOut() {
        StepResult out;
        if (state_.runState != DebugRunState::Paused) {
            out.ok = false;
            out.diagnostics.push_back("must_be_paused");
            out.state = state_;
            return out;
        }
        if (state_.stackDepth <= 1) {
            state_.runState = DebugRunState::Completed;
            state_.lastCommand = "step_out";
            out.ok = true;
            out.state = state_;
            out.diagnostics.push_back("session_completed");
            return out;
        }
        state_.stackDepth -= 1;
        state_.instructionPointer += 1;
        state_.lastCommand = "step_out";
        out.ok = true;
        out.state = state_;
        return out;
    }
};
