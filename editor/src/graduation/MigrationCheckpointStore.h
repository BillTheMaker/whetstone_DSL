#pragma once
// Step 906: Migration checkpoint store — saves and restores migration checkpoints.
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct MigrationCheckpoint {
    std::string checkpointId;   // "CP-<N>"
    std::string pairId;
    int stepIndex = 0;
    std::string state;          // "saved" | "restored" | "expired"
};

class MigrationCheckpointStore {
    std::unordered_map<std::string, MigrationCheckpoint> store_;
    int nextId_ = 1;

public:
    // Persist a checkpoint for pairId at stepIndex. Returns the created checkpoint.
    MigrationCheckpoint save(const std::string& pairId, int stepIndex) {
        MigrationCheckpoint cp;
        cp.checkpointId = "CP-" + std::to_string(nextId_++);
        cp.pairId       = pairId;
        cp.stepIndex    = stepIndex;
        cp.state        = "saved";
        store_[cp.checkpointId] = cp;
        return cp;
    }

    // Restore a checkpoint by ID: sets state to "restored". Returns false if not found.
    bool restore(const std::string& checkpointId) {
        auto it = store_.find(checkpointId);
        if (it == store_.end()) return false;
        if (it->second.state == "expired") return false;
        it->second.state = "restored";
        return true;
    }

    // Expire a checkpoint by ID. Returns false if not found.
    bool expire(const std::string& checkpointId) {
        auto it = store_.find(checkpointId);
        if (it == store_.end()) return false;
        it->second.state = "expired";
        return true;
    }

    bool has(const std::string& checkpointId) const {
        return store_.count(checkpointId) > 0;
    }

    const MigrationCheckpoint* get(const std::string& checkpointId) const {
        auto it = store_.find(checkpointId);
        return (it != store_.end()) ? &it->second : nullptr;
    }

    std::size_t count() const { return store_.size(); }

    void clear() {
        store_.clear();
        nextId_ = 1;
    }

    static nlohmann::json toJson(const MigrationCheckpoint& cp) {
        return {
            {"checkpoint_id", cp.checkpointId},
            {"pair_id",       cp.pairId},
            {"step_index",    cp.stepIndex},
            {"state",         cp.state}
        };
    }
};
