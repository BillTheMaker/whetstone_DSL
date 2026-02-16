// Step 381: Workflow progress + ETA tracking (12 tests)

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include "WorkflowProgress.h"

static OrchestratorEvent eventOf(const std::string& type,
                                 const std::string& itemId,
                                 const std::string& timestamp,
                                 const json& detail = json::object()) {
    OrchestratorEvent ev;
    ev.type = type;
    ev.itemId = itemId;
    ev.timestamp = timestamp;
    ev.detail = detail;
    return ev;
}

int main() {
    int passed = 0;

    // Test 1: completion percent tracks completed/total
    {
        WorkflowProgress progress(4);
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:00:00Z"));
        progress.recordEvent(eventOf("completed", "b", "2026-02-16T10:01:00Z"));
        auto snap = progress.getSnapshot();
        assert(snap.totalItems == 4);
        assert(snap.completedItems == 2);
        assert(std::fabs(snap.completionPercent - 50.0f) < 0.01f);
        std::cout << "Test 1 PASSED: completion percent accurate\n";
        passed++;
    }

    // Test 2: throughput computed from elapsed time and completions
    {
        WorkflowProgress progress(3);
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:00:00Z"));
        progress.recordEvent(eventOf("completed", "b", "2026-02-16T10:02:00Z"));
        auto snap = progress.getSnapshot();
        assert(std::fabs(snap.itemsPerMinute - 1.0f) < 0.01f);
        std::cout << "Test 2 PASSED: throughput computed\n";
        passed++;
    }

    // Test 3: ETA is close for uniform completion cadence
    {
        WorkflowProgress progress(4);
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:00:00Z"));
        progress.recordEvent(eventOf("completed", "b", "2026-02-16T10:02:00Z"));
        auto snap = progress.getSnapshot();
        // 2 done in 2 minutes => 1/min; remaining 2 => ~2 minutes.
        assert(snap.estimatedRemainingMinutes >= 1.0f);
        assert(snap.estimatedRemainingMinutes <= 4.0f);
        std::cout << "Test 3 PASSED: ETA stays within reasonable bound\n";
        passed++;
    }

    // Test 4: worker stats tracked per worker type
    {
        WorkflowProgress progress(2);
        progress.recordEvent(eventOf("routed", "a", "2026-02-16T10:00:00Z",
                                     json{{"workerType", "template"}}));
        progress.recordEvent(eventOf("executed", "a", "2026-02-16T10:00:10Z",
                                     json{{"workerType", "template"},
                                          {"confidence", 0.9},
                                          {"tokensGenerated", 40}}));
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:00:20Z"));
        progress.recordEvent(eventOf("routed", "b", "2026-02-16T10:00:30Z",
                                     json{{"workerType", "llm"}}));
        progress.recordEvent(eventOf("executed", "b", "2026-02-16T10:01:00Z",
                                     json{{"workerType", "llm"},
                                          {"confidence", 0.6},
                                          {"tokensGenerated", 200}}));
        auto byWorker = progress.getWorkerStats();
        assert(byWorker.count("template") == 1);
        assert(byWorker.count("llm") == 1);
        assert(byWorker["template"].completed == 1);
        assert(byWorker["llm"].completed == 0);
        std::cout << "Test 4 PASSED: worker stats grouped by worker type\n";
        passed++;
    }

    // Test 5: rejection rate updates when rejected events are recorded
    {
        WorkflowProgress progress(1);
        progress.recordEvent(eventOf("routed", "a", "2026-02-16T10:00:00Z",
                                     json{{"workerType", "slm"}}));
        progress.recordEvent(eventOf("executed", "a", "2026-02-16T10:00:20Z",
                                     json{{"workerType", "slm"},
                                          {"confidence", 0.7},
                                          {"tokensGenerated", 80}}));
        progress.recordEvent(eventOf("rejected", "a", "2026-02-16T10:00:25Z"));
        auto stats = progress.getWorkerStats();
        assert(stats.count("slm") == 1);
        assert(std::fabs(stats["slm"].rejectionRate - 1.0f) < 0.01f);
        std::cout << "Test 5 PASSED: rejection rate tracked\n";
        passed++;
    }

    // Test 6: timeline records all events in order
    {
        WorkflowProgress progress(1);
        progress.recordEvent(eventOf("routed", "a", "2026-02-16T10:00:00Z"));
        progress.recordEvent(eventOf("executed", "a", "2026-02-16T10:00:10Z"));
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:00:20Z"));
        auto timeline = progress.getTimeline();
        assert(timeline.size() == 3);
        assert(timeline[0].type == "routed");
        assert(timeline[2].type == "completed");
        std::cout << "Test 6 PASSED: timeline event log preserved\n";
        passed++;
    }

    // Test 7: empty snapshot shows zero progress and unknown ETA
    {
        WorkflowProgress progress;
        auto snap = progress.getSnapshot();
        assert(snap.completionPercent == 0.0f);
        assert(snap.estimatedRemainingMinutes < 0.0f);
        std::cout << "Test 7 PASSED: empty workflow snapshot sane\n";
        passed++;
    }

    // Test 8: blockers included from blocked event payload
    {
        WorkflowProgress progress(2);
        progress.recordEvent(eventOf("blocked", "", "2026-02-16T10:00:00Z",
                                     json{{"blockers", json::array({
                                         json{{"type", "needs-human"},
                                              {"itemIds", json::array({"x"})},
                                              {"description", "manual"}}})}}));
        auto snap = progress.getSnapshot();
        assert(snap.blockers.size() == 1);
        assert(snap.blockers[0].type == "needs-human");
        std::cout << "Test 8 PASSED: blockers surfaced in snapshot\n";
        passed++;
    }

    // Test 9: snapshot JSON serialization has expected fields
    {
        WorkflowProgress progress(1);
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:00:00Z"));
        auto j = progress.getSnapshot().toJson();
        assert(j.contains("totalItems"));
        assert(j.contains("completionPercent"));
        assert(j.contains("byWorkerType"));
        assert(j.contains("blockers"));
        std::cout << "Test 9 PASSED: snapshot serializes to JSON\n";
        passed++;
    }

    // Test 10: completion percent stays monotonic under rejections
    {
        WorkflowProgress progress(3);
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:00:00Z"));
        float p1 = progress.getSnapshot().completionPercent;
        progress.recordEvent(eventOf("rejected", "b", "2026-02-16T10:00:10Z"));
        float p2 = progress.getSnapshot().completionPercent;
        progress.recordEvent(eventOf("completed", "b", "2026-02-16T10:01:00Z"));
        float p3 = progress.getSnapshot().completionPercent;
        assert(p2 >= p1);
        assert(p3 >= p2);
        std::cout << "Test 10 PASSED: completion stays monotonic\n";
        passed++;
    }

    // Test 11: startedAt and lastActivityAt are tracked
    {
        WorkflowProgress progress(1);
        progress.recordEvent(eventOf("routed", "a", "2026-02-16T10:00:00Z"));
        progress.recordEvent(eventOf("completed", "a", "2026-02-16T10:02:00Z"));
        auto snap = progress.getSnapshot();
        assert(snap.startedAt == "2026-02-16T10:00:00Z");
        assert(snap.lastActivityAt == "2026-02-16T10:02:00Z");
        std::cout << "Test 11 PASSED: activity timestamps tracked\n";
        passed++;
    }

    // Test 12: total items fallback to observed unique item ids
    {
        WorkflowProgress progress;
        progress.recordEvent(eventOf("routed", "a", "2026-02-16T10:00:00Z"));
        progress.recordEvent(eventOf("routed", "b", "2026-02-16T10:00:10Z"));
        auto snap = progress.getSnapshot();
        assert(snap.totalItems == 2);
        std::cout << "Test 12 PASSED: observed-item fallback for total count\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
