#pragma once
// Step 881: Parallel dispatch optimizer with dependency constraints.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct DispatchSlot {
    std::string slotId;
    std::string pairId;
    bool scheduled = false;
};

struct DispatchPlan {
    std::vector<DispatchSlot> slots;
    int maxParallelism = 0;
    int scheduledCount = 0;
};

class ParallelDispatchOptimizer {
public:
    static DispatchPlan optimize(const std::vector<std::string>& pairIds,
                                  int maxParallelism,
                                  const std::vector<std::string>& blockedPairs = {}) {
        DispatchPlan plan;
        plan.maxParallelism = maxParallelism;
        int idx = 0;
        for (const auto& pid : pairIds) {
            bool blocked = false;
            for (const auto& b : blockedPairs) if (b == pid) { blocked = true; break; }
            DispatchSlot slot;
            slot.slotId = "slot-" + std::to_string(++idx);
            slot.pairId = pid;
            slot.scheduled = !blocked && plan.scheduledCount < maxParallelism;
            if (slot.scheduled) ++plan.scheduledCount;
            plan.slots.push_back(slot);
        }
        return plan;
    }

    static nlohmann::json toJson(const DispatchPlan& p) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& s : p.slots)
            arr.push_back({{"slot_id", s.slotId}, {"pair_id", s.pairId},
                           {"scheduled", s.scheduled}});
        return {{"max_parallelism", p.maxParallelism},
                {"scheduled_count", p.scheduledCount}, {"slots", arr}};
    }
};
