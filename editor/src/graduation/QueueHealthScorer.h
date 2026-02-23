#pragma once
// Step 883: Queue health scoring model.
#include <string>
#include <nlohmann/json.hpp>

struct QueueHealthScore {
    std::string queueId;
    float score = 0.0f;  // 0.0 - 1.0
    std::string label;   // "healthy", "degraded", "critical"
    int activeCount = 0;
    int stalledCount = 0;
};

class QueueHealthScorer {
public:
    static QueueHealthScore score(const std::string& queueId,
                                   int activeCount, int stalledCount,
                                   int totalCapacity) {
        QueueHealthScore h;
        h.queueId = queueId;
        h.activeCount = activeCount;
        h.stalledCount = stalledCount;
        float stallRate = (activeCount + stalledCount) > 0
            ? static_cast<float>(stalledCount) / (activeCount + stalledCount) : 0.0f;
        float utilRate = totalCapacity > 0
            ? static_cast<float>(activeCount) / totalCapacity : 0.0f;
        h.score = std::max(0.0f, 1.0f - stallRate * 0.7f - (utilRate > 0.9f ? 0.3f : 0.0f));
        if (h.score >= 0.7f)      h.label = "healthy";
        else if (h.score >= 0.4f) h.label = "degraded";
        else                      h.label = "critical";
        return h;
    }

    static nlohmann::json toJson(const QueueHealthScore& h) {
        return {{"queue_id", h.queueId}, {"score", h.score}, {"label", h.label},
                {"active_count", h.activeCount}, {"stalled_count", h.stalledCount}};
    }
};
