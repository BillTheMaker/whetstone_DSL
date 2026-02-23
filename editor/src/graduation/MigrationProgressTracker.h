#pragma once
// Step 905: Migration progress tracker — tracks per-pair step completion state.
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct MigrationProgress {
    std::string pairId;
    int totalSteps     = 0;
    int completedSteps = 0;
    std::string status;   // "pending" | "in_progress" | "complete" | "failed"
};

class MigrationProgressTracker {
    std::unordered_map<std::string, MigrationProgress> records_;

public:
    // Initialise tracking for a pair. Status becomes "pending".
    void start(const std::string& pairId, int totalSteps) {
        MigrationProgress p;
        p.pairId         = pairId;
        p.totalSteps     = totalSteps > 0 ? totalSteps : 1;
        p.completedSteps = 0;
        p.status         = "pending";
        records_[pairId] = p;
    }

    // Increment completedSteps by 1. Sets status to "in_progress" if not already complete.
    void advance(const std::string& pairId) {
        auto it = records_.find(pairId);
        if (it == records_.end()) return;
        auto& p = it->second;
        if (p.status == "complete" || p.status == "failed") return;
        p.completedSteps++;
        p.status = "in_progress";
        if (p.completedSteps >= p.totalSteps) p.status = "complete";
    }

    // Force status to "complete".
    void complete(const std::string& pairId) {
        auto it = records_.find(pairId);
        if (it == records_.end()) return;
        it->second.completedSteps = it->second.totalSteps;
        it->second.status = "complete";
    }

    // Force status to "failed".
    void fail(const std::string& pairId) {
        auto it = records_.find(pairId);
        if (it == records_.end()) return;
        it->second.status = "failed";
    }

    // Returns pointer to progress record, or nullptr if pair not tracked.
    const MigrationProgress* get(const std::string& pairId) const {
        auto it = records_.find(pairId);
        return (it != records_.end()) ? &it->second : nullptr;
    }

    // Returns completion percentage [0.0, 100.0], or -1.0 if pair not tracked.
    float percentComplete(const std::string& pairId) const {
        const auto* p = get(pairId);
        if (!p || p->totalSteps == 0) return -1.0f;
        return (static_cast<float>(p->completedSteps) /
                static_cast<float>(p->totalSteps)) * 100.0f;
    }

    std::size_t count() const { return records_.size(); }

    static nlohmann::json toJson(const MigrationProgress& p) {
        return {
            {"pair_id",         p.pairId},
            {"total_steps",     p.totalSteps},
            {"completed_steps", p.completedSteps},
            {"status",          p.status}
        };
    }
};
