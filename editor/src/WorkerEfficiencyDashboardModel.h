#pragma once
// Step 550: Worker Efficiency Dashboard Data Model

#include <map>
#include <string>
#include <vector>

struct WorkerRunRecord {
    std::string workerId;
    std::string taskClass;
    bool success = false;
    int tokensUsed = 0;
    int latencyMs = 0;
    int rejectionCount = 0;
};

struct WorkerTaskMetrics {
    int runs = 0;
    int successCount = 0;
    int failureCount = 0;
    int totalTokens = 0;
    int totalLatencyMs = 0;
    int totalRejections = 0;
    double successRate = 0.0;
    double avgTokens = 0.0;
    double avgLatencyMs = 0.0;
    double avgRejections = 0.0;
};

struct WorkerEfficiencySnapshot {
    std::map<std::string, WorkerTaskMetrics> byWorker;
    std::map<std::string, WorkerTaskMetrics> byTaskClass;
};

class WorkerEfficiencyDashboardModel {
public:
    void record(const WorkerRunRecord& r) { records_.push_back(r); }

    WorkerEfficiencySnapshot snapshot() const {
        WorkerEfficiencySnapshot out;
        for (const auto& r : records_) {
            accumulate(out.byWorker[r.workerId], r);
            accumulate(out.byTaskClass[r.taskClass], r);
        }
        finalize(out.byWorker);
        finalize(out.byTaskClass);
        return out;
    }

private:
    std::vector<WorkerRunRecord> records_;

    static void accumulate(WorkerTaskMetrics& m, const WorkerRunRecord& r) {
        ++m.runs;
        if (r.success) ++m.successCount; else ++m.failureCount;
        m.totalTokens += r.tokensUsed;
        m.totalLatencyMs += r.latencyMs;
        m.totalRejections += r.rejectionCount;
    }

    static void finalize(std::map<std::string, WorkerTaskMetrics>& table) {
        for (auto& kv : table) {
            auto& m = kv.second;
            if (m.runs == 0) continue;
            m.successRate = static_cast<double>(m.successCount) / static_cast<double>(m.runs);
            m.avgTokens = static_cast<double>(m.totalTokens) / static_cast<double>(m.runs);
            m.avgLatencyMs = static_cast<double>(m.totalLatencyMs) / static_cast<double>(m.runs);
            m.avgRejections = static_cast<double>(m.totalRejections) / static_cast<double>(m.runs);
        }
    }
};
