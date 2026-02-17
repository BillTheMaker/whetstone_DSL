#pragma once
// Step 590: Crash + Recovery Reliability Sweep

#include <map>
#include <string>
#include <vector>

struct RecoverySessionSnapshot {
    std::string sessionId;
    std::string workflowId;
    int checkpoint = 0;
    std::vector<std::string> openBuffers;
    std::vector<std::string> queuedTaskIds;
};

struct RecoveryResult {
    bool recovered = false;
    bool workflowContinuity = false;
    bool queueContinuity = false;
    bool bufferContinuity = false;
    std::vector<std::string> notes;
};

class CrashRecoveryReliabilitySweep {
public:
    bool captureSnapshot(const RecoverySessionSnapshot& snapshot, std::string* error) {
        if (!error) return false;
        error->clear();
        if (snapshot.sessionId.empty()) return fail(error, "session_id_missing");
        if (snapshot.workflowId.empty()) return fail(error, "workflow_id_missing");
        if (snapshot.checkpoint <= 0) return fail(error, "checkpoint_invalid");
        snapshots_[snapshot.sessionId] = snapshot;
        return true;
    }

    RecoveryResult recover(const std::string& sessionId) const {
        RecoveryResult result;
        auto it = snapshots_.find(sessionId);
        if (it == snapshots_.end()) {
            result.notes.push_back("recover:session_missing");
            return result;
        }
        const auto& s = it->second;
        result.workflowContinuity = !s.workflowId.empty() && s.checkpoint > 0;
        result.queueContinuity = !s.queuedTaskIds.empty();
        result.bufferContinuity = !s.openBuffers.empty();
        result.recovered = result.workflowContinuity && result.queueContinuity && result.bufferContinuity;

        if (result.recovered) {
            result.notes.push_back("recover:ok");
        } else {
            result.notes.push_back("recover:partial");
            if (!result.workflowContinuity) result.notes.push_back("recover:workflow_missing");
            if (!result.queueContinuity) result.notes.push_back("recover:queue_missing");
            if (!result.bufferContinuity) result.notes.push_back("recover:buffers_missing");
        }
        return result;
    }

    std::vector<std::string> knownSessions() const {
        std::vector<std::string> ids;
        for (const auto& kv : snapshots_) ids.push_back(kv.first);
        return ids;
    }

private:
    std::map<std::string, RecoverySessionSnapshot> snapshots_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }
};
