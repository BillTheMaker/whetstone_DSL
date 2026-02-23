#pragma once
// Step 887: Upgrade queue observability panel model.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct QueuePanelSnapshot {
    std::string snapshotId;
    int totalQueued = 0;
    int totalActive = 0;
    int totalStalled = 0;
    int totalCompleted = 0;
    float healthScore = 0.0f;
    std::string healthLabel;
};

class UpgradeQueueObservabilityPanel {
public:
    static QueuePanelSnapshot snapshot(const std::string& snapshotId,
                                        int queued, int active,
                                        int stalled, int completed,
                                        float healthScore) {
        QueuePanelSnapshot s;
        s.snapshotId = snapshotId;
        s.totalQueued = queued;
        s.totalActive = active;
        s.totalStalled = stalled;
        s.totalCompleted = completed;
        s.healthScore = healthScore;
        if (healthScore >= 0.7f)      s.healthLabel = "healthy";
        else if (healthScore >= 0.4f) s.healthLabel = "degraded";
        else                          s.healthLabel = "critical";
        return s;
    }

    static nlohmann::json toJson(const QueuePanelSnapshot& s) {
        return {{"snapshot_id", s.snapshotId},
                {"total_queued", s.totalQueued},
                {"total_active", s.totalActive},
                {"total_stalled", s.totalStalled},
                {"total_completed", s.totalCompleted},
                {"health_score", s.healthScore},
                {"health_label", s.healthLabel}};
    }
};
