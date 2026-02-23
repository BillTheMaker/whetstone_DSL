#pragma once
// Step 879: Pair-upgrade queue model.
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>

struct UpgradeQueueEntry {
    std::string entryId;
    std::string pairId;
    std::string priority; // "critical","revenue","coverage","experimental"
    int priorityLevel = 0; // higher = more urgent
    bool active = true;
};

class PairUpgradeQueue {
public:
    bool enqueue(const UpgradeQueueEntry& e, std::string* error = nullptr) {
        if (e.pairId.empty()) { if (error) *error = "pair_id_missing"; return false; }
        if (e.entryId.empty()) { if (error) *error = "entry_id_missing"; return false; }
        entries_.push_back(e);
        return true;
    }

    bool dequeue(const std::string& entryId) {
        for (auto& e : entries_)
            if (e.entryId == entryId && e.active) { e.active = false; return true; }
        return false;
    }

    int activeCount() const {
        int n = 0; for (const auto& e : entries_) if (e.active) ++n; return n;
    }

    std::vector<UpgradeQueueEntry> getActive() const {
        std::vector<UpgradeQueueEntry> res;
        for (const auto& e : entries_) if (e.active) res.push_back(e);
        std::stable_sort(res.begin(), res.end(),
            [](const UpgradeQueueEntry& a, const UpgradeQueueEntry& b){
                return a.priorityLevel > b.priorityLevel; });
        return res;
    }

    static nlohmann::json toJson(const UpgradeQueueEntry& e) {
        return {{"entry_id", e.entryId}, {"pair_id", e.pairId},
                {"priority", e.priority}, {"priority_level", e.priorityLevel},
                {"active", e.active}};
    }

private:
    std::vector<UpgradeQueueEntry> entries_;
};
