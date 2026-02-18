#pragma once
// Step 655: Job status subscriber panel model

#include <string>
#include <vector>

enum class SwarmJobState { Pending, Running, Completed, Failed };

struct SwarmJobStatusItem {
    std::string jobId;
    std::string node;
    int runningSeconds = 0;
    SwarmJobState state = SwarmJobState::Pending;
};

struct SwarmStatusSummary {
    int pending = 0;
    int running = 0;
    int completed = 0;
    int failed = 0;
};

class SwarmStatusPanelModel {
public:
    static SwarmStatusSummary summarize(const std::vector<SwarmJobStatusItem>& items) {
        SwarmStatusSummary s;
        for (const auto& i : items) {
            if (i.state == SwarmJobState::Pending) ++s.pending;
            else if (i.state == SwarmJobState::Running) ++s.running;
            else if (i.state == SwarmJobState::Completed) ++s.completed;
            else ++s.failed;
        }
        return s;
    }

    static std::string label(const SwarmJobStatusItem& item) {
        return item.jobId + "@" + item.node;
    }

    static std::string stateText(SwarmJobState state) {
        if (state == SwarmJobState::Pending) return "pending";
        if (state == SwarmJobState::Running) return "running";
        if (state == SwarmJobState::Completed) return "completed";
        return "failed";
    }
};
