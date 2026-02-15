#pragma once
// Step 322: WorkflowState — Project-Level Workflow Tracking
//
// Top-level state managing the full workflow lifecycle for a project: from
// skeleton creation through routing, execution, review, and completion.

#include "TaskQueue.h"
#include "SkeletonAST.h"
#include <string>
#include <vector>
#include <map>

// --- WorkflowPhase ---

enum class WorkflowPhase {
    Modeling,    // architect is creating/annotating skeletons
    Routing,     // routing engine is assigning worker types
    Executing,   // workers are producing results
    Reviewing,   // human review in progress
    Complete     // all work items complete
};

inline std::string workflowPhaseToString(WorkflowPhase p) {
    switch (p) {
        case WorkflowPhase::Modeling:  return "modeling";
        case WorkflowPhase::Routing:   return "routing";
        case WorkflowPhase::Executing: return "executing";
        case WorkflowPhase::Reviewing: return "reviewing";
        case WorkflowPhase::Complete:  return "complete";
    }
    return "unknown";
}

inline WorkflowPhase workflowPhaseFromString(const std::string& s) {
    if (s == "modeling")  return WorkflowPhase::Modeling;
    if (s == "routing")   return WorkflowPhase::Routing;
    if (s == "executing") return WorkflowPhase::Executing;
    if (s == "reviewing") return WorkflowPhase::Reviewing;
    if (s == "complete")  return WorkflowPhase::Complete;
    return WorkflowPhase::Modeling;
}

// --- StatusChange: audit trail entry ---

struct StatusChange {
    std::string itemId;
    std::string fromStatus;
    std::string toStatus;
    std::string timestamp;
    std::string actor;
    std::string note;

    json toJson() const {
        return json{
            {"itemId", itemId}, {"fromStatus", fromStatus},
            {"toStatus", toStatus}, {"timestamp", timestamp},
            {"actor", actor}, {"note", note}
        };
    }

    static StatusChange fromJson(const json& j) {
        StatusChange sc;
        if (j.contains("itemId")) sc.itemId = j["itemId"].get<std::string>();
        if (j.contains("fromStatus")) sc.fromStatus = j["fromStatus"].get<std::string>();
        if (j.contains("toStatus")) sc.toStatus = j["toStatus"].get<std::string>();
        if (j.contains("timestamp")) sc.timestamp = j["timestamp"].get<std::string>();
        if (j.contains("actor")) sc.actor = j["actor"].get<std::string>();
        if (j.contains("note")) sc.note = j["note"].get<std::string>();
        return sc;
    }
};

// --- WorkflowStats ---

struct WorkflowStats {
    int total = 0;
    int pending = 0;
    int ready = 0;
    int assigned = 0;
    int inProgress = 0;
    int review = 0;
    int complete = 0;
    int rejected = 0;
    std::map<std::string, int> byWorkerType;
    std::map<std::string, int> byPriority;
    float completionPercent = 0.0f;

    json toJson() const {
        json j;
        j["total"] = total;
        j["pending"] = pending;
        j["ready"] = ready;
        j["assigned"] = assigned;
        j["inProgress"] = inProgress;
        j["review"] = review;
        j["complete"] = complete;
        j["rejected"] = rejected;
        j["byWorkerType"] = byWorkerType;
        j["byPriority"] = byPriority;
        j["completionPercent"] = completionPercent;
        return j;
    }
};

// --- WorkflowState ---

class WorkflowState {
public:
    TaskQueue queue;
    std::string projectName;
    std::string createdAt;

    WorkflowState() : createdAt(workItemTimestamp()) {}

    explicit WorkflowState(const std::string& name)
        : projectName(name), createdAt(workItemTimestamp()) {}

    // Walk skeleton AST, create WorkItems, enqueue all. Returns count.
    int populateFromSkeleton(const ASTNode* module, const std::string& bufferId) {
        auto tasks = skeletonToTaskList(module);
        int count = 0;
        for (const auto& task : tasks) {
            WorkItem wi = createWorkItem(task, bufferId);
            recordChange(wi.id, "", WI_PENDING, "system", "created from skeleton");
            queue.enqueue(wi);
            ++count;
        }
        return count;
    }

    // Get workflow statistics
    WorkflowStats getStats() const {
        WorkflowStats stats;
        stats.total = queue.size();

        auto count = [&](const std::string& status) {
            return static_cast<int>(queue.getByStatus(status).size());
        };
        stats.pending = count(WI_PENDING);
        stats.ready = count(WI_READY);
        stats.assigned = count(WI_ASSIGNED);
        stats.inProgress = count(WI_IN_PROGRESS);
        stats.review = count(WI_REVIEW);
        stats.complete = count(WI_COMPLETE);
        stats.rejected = count(WI_REJECTED);

        // Aggregate by worker type and priority
        for (const auto& status : {WI_PENDING, WI_READY, WI_ASSIGNED, WI_IN_PROGRESS,
                                    WI_REVIEW, WI_COMPLETE, WI_REJECTED}) {
            for (const auto& item : queue.getByStatus(status)) {
                if (!item.workerType.empty()) stats.byWorkerType[item.workerType]++;
                if (!item.priority.empty()) stats.byPriority[item.priority]++;
            }
        }

        if (stats.total > 0) {
            stats.completionPercent = 100.0f * static_cast<float>(stats.complete)
                                     / static_cast<float>(stats.total);
        }
        return stats;
    }

    // Auto-compute phase from item statuses
    WorkflowPhase getPhase() const {
        if (queue.size() == 0) return WorkflowPhase::Modeling;

        auto stats = getStats();

        // All complete → Complete
        if (stats.complete == stats.total) return WorkflowPhase::Complete;

        // Any in review → Reviewing
        if (stats.review > 0) return WorkflowPhase::Reviewing;

        // Any assigned or in-progress → Executing
        if (stats.assigned > 0 || stats.inProgress > 0) return WorkflowPhase::Executing;

        // Any ready (routed) → Routing
        if (stats.ready > 0) return WorkflowPhase::Routing;

        // All pending → Modeling
        return WorkflowPhase::Modeling;
    }

    // Audit trail for a specific item
    std::vector<StatusChange> getHistory(const std::string& itemId) const {
        std::vector<StatusChange> result;
        for (const auto& sc : history_) {
            if (sc.itemId == itemId) result.push_back(sc);
        }
        return result;
    }

    // Record a status change in the audit trail
    void recordChange(const std::string& itemId, const std::string& from,
                      const std::string& to, const std::string& actor,
                      const std::string& note = "") {
        StatusChange sc;
        sc.itemId = itemId;
        sc.fromStatus = from;
        sc.toStatus = to;
        sc.timestamp = workItemTimestamp();
        sc.actor = actor;
        sc.note = note;
        history_.push_back(sc);
    }

    // Full state serialization
    json toJson() const {
        json j;
        j["projectName"] = projectName;
        j["createdAt"] = createdAt;
        j["phase"] = workflowPhaseToString(getPhase());

        // Serialize all items from the queue
        json items = json::array();
        for (const auto& status : {WI_PENDING, WI_READY, WI_ASSIGNED, WI_IN_PROGRESS,
                                    WI_REVIEW, WI_COMPLETE, WI_REJECTED}) {
            for (const auto& item : queue.getByStatus(status)) {
                items.push_back(workItemToJson(item));
            }
        }
        j["items"] = items;

        // Serialize history
        json hist = json::array();
        for (const auto& sc : history_) {
            hist.push_back(sc.toJson());
        }
        j["history"] = hist;

        j["stats"] = getStats().toJson();
        return j;
    }

    // Deserialize from JSON
    static WorkflowState fromJson(const json& j) {
        WorkflowState ws;
        if (j.contains("projectName")) ws.projectName = j["projectName"].get<std::string>();
        if (j.contains("createdAt")) ws.createdAt = j["createdAt"].get<std::string>();

        if (j.contains("items")) {
            for (const auto& itemJ : j["items"]) {
                WorkItem wi = workItemFromJson(itemJ);
                ws.queue.enqueue(wi);
            }
        }

        if (j.contains("history")) {
            for (const auto& scJ : j["history"]) {
                ws.history_.push_back(StatusChange::fromJson(scJ));
            }
        }
        return ws;
    }

private:
    std::vector<StatusChange> history_;
};
