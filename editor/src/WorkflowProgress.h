#pragma once
// Step 381: Workflow progress + ETA tracking
//
// Aggregates orchestrator events into live workflow metrics for completion
// tracking, throughput, ETA, worker performance, and visualization timeline.

#include "WorkflowOrchestrator.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <map>
#include <set>
#include <string>
#include <vector>

struct WorkerStats {
    int completed = 0;
    float avgConfidence = 0.0f;
    int avgTokensUsed = 0;
    float rejectionRate = 0.0f;

    json toJson() const {
        return json{
            {"completed", completed},
            {"avgConfidence", avgConfidence},
            {"avgTokensUsed", avgTokensUsed},
            {"rejectionRate", rejectionRate}
        };
    }
};

struct TimelineEntry {
    std::string type;
    std::string itemId;
    std::string timestamp;
    json detail;

    json toJson() const {
        return json{
            {"type", type},
            {"itemId", itemId},
            {"timestamp", timestamp},
            {"detail", detail}
        };
    }
};

struct ProgressSnapshot {
    int totalItems = 0;
    int completedItems = 0;
    float completionPercent = 0.0f;
    float itemsPerMinute = 0.0f;
    float estimatedRemainingMinutes = -1.0f; // -1 = unknown/unavailable
    std::map<std::string, WorkerStats> byWorkerType;
    std::string currentPhase;
    std::vector<BlockerInfo> blockers;
    std::string startedAt;
    std::string lastActivityAt;

    json toJson() const {
        json workers = json::object();
        for (const auto& [k, v] : byWorkerType) {
            workers[k] = v.toJson();
        }

        json blockerArr = json::array();
        for (const auto& b : blockers) {
            blockerArr.push_back(json{
                {"type", b.type},
                {"itemIds", b.itemIds},
                {"description", b.description}
            });
        }

        return json{
            {"totalItems", totalItems},
            {"completedItems", completedItems},
            {"completionPercent", completionPercent},
            {"itemsPerMinute", itemsPerMinute},
            {"estimatedRemainingMinutes", estimatedRemainingMinutes},
            {"byWorkerType", workers},
            {"currentPhase", currentPhase},
            {"blockers", blockerArr},
            {"startedAt", startedAt},
            {"lastActivityAt", lastActivityAt}
        };
    }
};

class WorkflowProgress {
public:
    explicit WorkflowProgress(int totalItems = 0) : totalItems_(std::max(0, totalItems)) {}

    void setTotalItems(int totalItems) {
        totalItems_ = std::max(0, totalItems);
    }

    void recordEvent(const OrchestratorEvent& event) {
        TimelineEntry entry{event.type, event.itemId, event.timestamp, event.detail};
        timeline_.push_back(entry);

        if (startedAt_.empty()) startedAt_ = event.timestamp;
        lastActivityAt_ = event.timestamp;

        if (event.type == "routed") {
            noteWorkerType(event.itemId, event.detail.value("workerType", ""));
            clearMatchingBlockers(event.itemId);
        } else if (event.type == "executed") {
            std::string worker = event.detail.value("workerType", "");
            noteWorkerType(event.itemId, worker);
            if (!worker.empty()) {
                workerAttempts_[worker]++;
                workerConfidenceSum_[worker] += event.detail.value("confidence", 0.0f);
                workerTokenSum_[worker] += event.detail.value("tokensGenerated", 0);
            }
            clearMatchingBlockers(event.itemId);
        } else if (event.type == "completed") {
            if (completedIds_.insert(event.itemId).second) {
                completedItems_++;
                std::string worker = workerForItem(event.itemId);
                if (!worker.empty()) workerCompleted_[worker]++;
            }
            clearMatchingBlockers(event.itemId);
        } else if (event.type == "rejected") {
            std::string worker = workerForItem(event.itemId);
            if (!worker.empty()) workerRejected_[worker]++;
        } else if (event.type == "blocked") {
            updateBlockersFromEvent(event.detail);
        }
    }

    ProgressSnapshot getSnapshot() const {
        ProgressSnapshot s;
        s.totalItems = totalItems_ > 0 ? totalItems_ : observedTotalItems();
        s.completedItems = completedItems_;
        s.completionPercent = completionPercent(s.totalItems, s.completedItems);
        s.itemsPerMinute = throughputPerMinute();
        int remaining = std::max(0, s.totalItems - s.completedItems);
        if (s.itemsPerMinute > 0.0f && remaining > 0) {
            s.estimatedRemainingMinutes = static_cast<float>(remaining) / s.itemsPerMinute;
        } else if (remaining == 0 && s.totalItems > 0) {
            s.estimatedRemainingMinutes = 0.0f;
        }
        s.byWorkerType = getWorkerStats();
        s.currentPhase = inferPhase(s);
        s.blockers = blockers_;
        s.startedAt = startedAt_;
        s.lastActivityAt = lastActivityAt_;
        return s;
    }

    std::vector<TimelineEntry> getTimeline() const {
        return timeline_;
    }

    std::map<std::string, WorkerStats> getWorkerStats() const {
        std::map<std::string, WorkerStats> out;
        std::set<std::string> workerKeys;
        for (const auto& [k, _] : workerAttempts_) workerKeys.insert(k);
        for (const auto& [k, _] : workerCompleted_) workerKeys.insert(k);
        for (const auto& [k, _] : workerRejected_) workerKeys.insert(k);

        for (const auto& worker : workerKeys) {
            WorkerStats ws;
            ws.completed = mapGet(workerCompleted_, worker);
            int attempts = mapGet(workerAttempts_, worker);
            if (attempts > 0) {
                ws.avgConfidence = mapGet(workerConfidenceSum_, worker) /
                                   static_cast<float>(attempts);
                ws.avgTokensUsed = mapGet(workerTokenSum_, worker) / attempts;
                ws.rejectionRate = static_cast<float>(mapGet(workerRejected_, worker)) /
                                   static_cast<float>(attempts);
            }
            out[worker] = ws;
        }
        return out;
    }

private:
    int totalItems_ = 0;
    int completedItems_ = 0;
    std::string startedAt_;
    std::string lastActivityAt_;
    std::vector<TimelineEntry> timeline_;
    std::vector<BlockerInfo> blockers_;
    std::map<std::string, std::string> itemWorkerType_;
    std::set<std::string> completedIds_;

    std::map<std::string, int> workerAttempts_;
    std::map<std::string, int> workerCompleted_;
    std::map<std::string, int> workerRejected_;
    std::map<std::string, float> workerConfidenceSum_;
    std::map<std::string, int> workerTokenSum_;

    static bool isDigits(const std::string& s) {
        for (char c : s) {
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        }
        return !s.empty();
    }

    static int toIntOrDefault(const std::string& s, int fallback) {
        if (!isDigits(s)) return fallback;
        return std::stoi(s);
    }

    static std::time_t parseIso8601(const std::string& ts) {
        // Expected format: YYYY-MM-DDTHH:MM:SSZ
        if (ts.size() < 20) return 0;
        std::tm t{};
        t.tm_year = toIntOrDefault(ts.substr(0, 4), 1970) - 1900;
        t.tm_mon = toIntOrDefault(ts.substr(5, 2), 1) - 1;
        t.tm_mday = toIntOrDefault(ts.substr(8, 2), 1);
        t.tm_hour = toIntOrDefault(ts.substr(11, 2), 0);
        t.tm_min = toIntOrDefault(ts.substr(14, 2), 0);
        t.tm_sec = toIntOrDefault(ts.substr(17, 2), 0);
        t.tm_isdst = 0;
        return timegm(&t);
    }

    static float completionPercent(int total, int completed) {
        if (total <= 0) return 0.0f;
        return 100.0f * static_cast<float>(completed) / static_cast<float>(total);
    }

    float throughputPerMinute() const {
        if (timeline_.size() < 2 || startedAt_.empty() || lastActivityAt_.empty()) {
            return 0.0f;
        }
        std::time_t start = parseIso8601(startedAt_);
        std::time_t end = parseIso8601(lastActivityAt_);
        if (end <= start) return 0.0f;
        double minutes = static_cast<double>(end - start) / 60.0;
        if (minutes <= 0.0) return 0.0f;
        return static_cast<float>(completedItems_ / minutes);
    }

    int observedTotalItems() const {
        std::set<std::string> itemIds;
        for (const auto& e : timeline_) {
            if (!e.itemId.empty()) itemIds.insert(e.itemId);
        }
        return static_cast<int>(itemIds.size());
    }

    void noteWorkerType(const std::string& itemId, const std::string& workerType) {
        if (!itemId.empty() && !workerType.empty()) {
            itemWorkerType_[itemId] = workerType;
        }
    }

    std::string workerForItem(const std::string& itemId) const {
        auto it = itemWorkerType_.find(itemId);
        if (it == itemWorkerType_.end()) return "";
        return it->second;
    }

    static int mapGet(const std::map<std::string, int>& m, const std::string& key) {
        auto it = m.find(key);
        if (it == m.end()) return 0;
        return it->second;
    }

    static float mapGet(const std::map<std::string, float>& m, const std::string& key) {
        auto it = m.find(key);
        if (it == m.end()) return 0.0f;
        return it->second;
    }

    void clearMatchingBlockers(const std::string& itemId) {
        for (auto& blocker : blockers_) {
            auto& ids = blocker.itemIds;
            ids.erase(std::remove(ids.begin(), ids.end(), itemId), ids.end());
        }
        blockers_.erase(std::remove_if(blockers_.begin(), blockers_.end(),
                                       [](const BlockerInfo& b) {
                                           return b.itemIds.empty();
                                       }),
                        blockers_.end());
    }

    void updateBlockersFromEvent(const json& detail) {
        if (!detail.contains("blockers") || !detail["blockers"].is_array()) return;
        blockers_.clear();
        for (const auto& b : detail["blockers"]) {
            BlockerInfo blocker;
            blocker.type = b.value("type", "");
            blocker.description = b.value("description", "");
            if (b.contains("itemIds") && b["itemIds"].is_array()) {
                blocker.itemIds = b["itemIds"].get<std::vector<std::string>>();
            }
            if (!blocker.type.empty()) blockers_.push_back(blocker);
        }
    }

    static std::string inferPhase(const ProgressSnapshot& s) {
        if (s.totalItems > 0 && s.completedItems >= s.totalItems) return "complete";
        if (!s.blockers.empty()) return "blocked";
        if (s.completedItems > 0) return "executing";
        if (s.totalItems > 0) return "running";
        return "idle";
    }
};
