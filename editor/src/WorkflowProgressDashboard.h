#pragma once

#include "WorkflowProgress.h"
#include "WorkflowCostTracker.h"

#include <map>
#include <string>
#include <vector>

struct ThroughputPoint {
    std::string timestamp;
    int completed = 0;
};

struct WorkerSlice {
    std::string workerType;
    int count = 0;
    float percent = 0.0f;
};

struct BlockerSummary {
    std::string type;
    int count = 0;
    std::string description;
};

struct ProgressDashboardModel {
    float completionPercent = 0.0f;
    int completedItems = 0;
    int totalItems = 0;
    float itemsPerMinute = 0.0f;
    float estimatedRemainingMinutes = -1.0f;
    std::vector<ThroughputPoint> throughputSeries;
    std::vector<WorkerSlice> workerBreakdown;
    std::vector<BlockerSummary> blockerSummary;
    double actualCost = 0.0;
    double estimatedCost = 0.0;
    double estimatedRemainingCost = 0.0;
};

class WorkflowProgressDashboard {
public:
    static ProgressDashboardModel build(const WorkflowState& wf,
                                        const WorkflowProgress& progress,
                                        const WorkflowCostTracker* costTracker = nullptr) {
        ProgressDashboardModel m;
        auto snapshot = progress.getSnapshot();
        m.completionPercent = snapshot.completionPercent;
        m.completedItems = snapshot.completedItems;
        m.totalItems = snapshot.totalItems;
        m.itemsPerMinute = snapshot.itemsPerMinute;
        m.estimatedRemainingMinutes = snapshot.estimatedRemainingMinutes;
        m.throughputSeries = buildThroughputSeries(progress.getTimeline());
        m.workerBreakdown = buildWorkerBreakdown(wf.getStats().byWorkerType);
        m.blockerSummary = buildBlockerSummary(snapshot.blockers);

        if (costTracker) {
            auto cost = costTracker->summarize();
            m.actualCost = cost.actualCost;
            m.estimatedCost = cost.estimatedCost;
            int remaining = std::max(0, m.totalItems - m.completedItems);
            double avgPerItem = 0.0;
            if (m.completedItems > 0) avgPerItem = m.actualCost / (double)m.completedItems;
            m.estimatedRemainingCost = avgPerItem * (double)remaining;
        }
        return m;
    }

private:
    static std::vector<ThroughputPoint> buildThroughputSeries(
        const std::vector<TimelineEntry>& timeline) {
        std::vector<ThroughputPoint> out;
        int completed = 0;
        for (const auto& e : timeline) {
            if (e.type == "completed") ++completed;
            out.push_back({e.timestamp, completed});
        }
        return out;
    }

    static std::vector<WorkerSlice> buildWorkerBreakdown(
        const std::map<std::string, int>& byWorkerType) {
        std::vector<WorkerSlice> out;
        int total = 0;
        for (const auto& [_, count] : byWorkerType) total += count;
        for (const auto& [worker, count] : byWorkerType) {
            WorkerSlice s;
            s.workerType = worker;
            s.count = count;
            s.percent = total > 0 ? (100.0f * (float)count / (float)total) : 0.0f;
            out.push_back(s);
        }
        std::sort(out.begin(), out.end(),
                  [](const WorkerSlice& a, const WorkerSlice& b) {
                      return a.workerType < b.workerType;
                  });
        return out;
    }

    static std::vector<BlockerSummary> buildBlockerSummary(
        const std::vector<BlockerInfo>& blockers) {
        std::vector<BlockerSummary> out;
        for (const auto& b : blockers) {
            out.push_back({b.type, (int)b.itemIds.size(), b.description});
        }
        std::sort(out.begin(), out.end(),
                  [](const BlockerSummary& a, const BlockerSummary& b) {
                      return a.type < b.type;
                  });
        return out;
    }
};
