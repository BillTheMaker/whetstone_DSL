#pragma once
// Step 387: Orchestrator event stream for polling/subscription.

#include "WorkflowOrchestrator.h"
#include <algorithm>
#include <functional>
#include <vector>

struct StreamEvent {
    int version = 0;
    OrchestratorEvent event;

    json toJson() const {
        return {
            {"version", version},
            {"type", event.type},
            {"itemId", event.itemId},
            {"detail", event.detail},
            {"timestamp", event.timestamp}
        };
    }
};

class EventStream {
public:
    using Callback = std::function<void(const StreamEvent&)>;

    void emit(const OrchestratorEvent& event) {
        StreamEvent se;
        se.version = ++version_;
        se.event = event;
        events_.push_back(se);
        for (const auto& cb : subscribers_) {
            cb(se);
        }
    }

    std::vector<StreamEvent> poll(int sinceVersion) const {
        std::vector<StreamEvent> out;
        for (const auto& se : events_) {
            if (se.version > sinceVersion) out.push_back(se);
        }
        return out;
    }

    void subscribe(Callback cb) {
        subscribers_.push_back(std::move(cb));
    }

    int getVersion() const { return version_; }

    std::vector<StreamEvent> getRecent(int count) const {
        if (count <= 0) return {};
        int n = std::min(count, static_cast<int>(events_.size()));
        return std::vector<StreamEvent>(events_.end() - n, events_.end());
    }

    static json toJson(const std::vector<StreamEvent>& events) {
        json arr = json::array();
        for (const auto& e : events) arr.push_back(e.toJson());
        return arr;
    }

private:
    int version_ = 0;
    std::vector<StreamEvent> events_;
    std::vector<Callback> subscribers_;
};
