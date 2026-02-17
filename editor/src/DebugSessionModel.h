#pragma once
// Step 554: Debug Session Model

#include <map>
#include <string>
#include <vector>

struct DebugTargetContext {
    std::string targetId;
    std::string bufferId;
    std::string executablePath;
    bool attached = false;
};

struct DebugSessionState {
    std::string sessionId;
    bool active = false;
    bool paused = false;
    std::map<std::string, DebugTargetContext> targets;
    std::string currentTargetId;
};

class DebugSessionModel {
public:
    DebugSessionModel() {
        state_.sessionId = "debug-session-1";
    }

    const DebugSessionState& state() const { return state_; }

    bool start(const std::string& targetId,
               const std::string& bufferId,
               const std::string& executablePath) {
        if (state_.active) return false;
        state_.active = true;
        state_.paused = false;
        state_.targets.clear();
        state_.currentTargetId.clear();
        return addTargetInternal(targetId, bufferId, executablePath, false, true);
    }

    bool attach(const std::string& targetId,
                const std::string& bufferId,
                const std::string& executablePath) {
        if (!state_.active) state_.active = true;
        return addTargetInternal(targetId, bufferId, executablePath, true,
                                 state_.currentTargetId.empty());
    }

    bool pause() {
        if (!state_.active || state_.paused) return false;
        state_.paused = true;
        return true;
    }

    bool resume() {
        if (!state_.active || !state_.paused) return false;
        state_.paused = false;
        return true;
    }

    bool stop() {
        if (!state_.active) return false;
        state_.active = false;
        state_.paused = false;
        state_.targets.clear();
        state_.currentTargetId.clear();
        return true;
    }

    bool switchTarget(const std::string& targetId) {
        if (!state_.active) return false;
        if (state_.targets.count(targetId) == 0) return false;
        state_.currentTargetId = targetId;
        return true;
    }

    bool hasTargetForBuffer(const std::string& bufferId) const {
        for (const auto& kv : state_.targets) {
            if (kv.second.bufferId == bufferId) return true;
        }
        return false;
    }

private:
    DebugSessionState state_;

    bool addTargetInternal(const std::string& targetId,
                           const std::string& bufferId,
                           const std::string& executablePath,
                           bool attached,
                           bool makeCurrent) {
        if (targetId.empty() || bufferId.empty() || executablePath.empty()) return false;
        if (state_.targets.count(targetId) != 0) return false;

        DebugTargetContext context;
        context.targetId = targetId;
        context.bufferId = bufferId;
        context.executablePath = executablePath;
        context.attached = attached;
        state_.targets[targetId] = context;

        if (makeCurrent) state_.currentTargetId = targetId;
        return true;
    }
};
