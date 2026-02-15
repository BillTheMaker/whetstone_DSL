// Step 322: WorkflowState — Project-Level Workflow Tracking (12 tests)
// Tests populateFromSkeleton, phase auto-detection, stats, history,
// JSON roundtrip, empty workflow, phase transitions.

#include "WorkflowState.h"
#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: build a skeleton module with N functions
static Module* makeSkeletonModule(const std::string& name, int funcCount) {
    auto* mod = createSkeletonModule(name, "python");
    for (int i = 0; i < funcCount; ++i) {
        addSkeletonFunction(mod, "func_" + std::to_string(i), {"x"}, "int");
    }
    return mod;
}

// 1. populateFromSkeleton creates correct item count
void test_populate_from_skeleton() {
    TEST(populate_from_skeleton);
    auto* mod = makeSkeletonModule("proj1", 5);
    WorkflowState ws("proj1");
    int count = ws.populateFromSkeleton(mod, "buf-1");
    CHECK(count == 5, "5 items created, got " + std::to_string(count));
    CHECK(ws.queue.size() == 5, "queue has 5 items");
    delete mod;
    PASS();
}

// 2. Phase auto-detection: empty → Modeling
void test_phase_empty() {
    TEST(phase_empty);
    WorkflowState ws("empty");
    CHECK(ws.getPhase() == WorkflowPhase::Modeling, "empty = Modeling");
    PASS();
}

// 3. Phase: all pending → Modeling
void test_phase_all_pending() {
    TEST(phase_all_pending);
    WorkflowState ws("proj");
    // Enqueue items with dependencies so they stay pending
    WorkItem a;
    a.id = "a"; a.nodeId = "n1"; a.nodeName = "f1"; a.nodeType = "Function";
    a.priority = "medium"; a.status = WI_PENDING;
    a.dependencies = {"external_dep"};
    a.createdAt = workItemTimestamp();
    ws.queue.enqueue(a);

    CHECK(ws.getPhase() == WorkflowPhase::Modeling, "all pending = Modeling");
    PASS();
}

// 4. Phase: some assigned → Executing
void test_phase_executing() {
    TEST(phase_executing);
    WorkflowState ws("proj");

    WorkItem a;
    a.id = "a1"; a.nodeId = "n1"; a.nodeName = "f1"; a.nodeType = "Function";
    a.priority = "medium"; a.status = WI_ASSIGNED;
    a.createdAt = workItemTimestamp();
    ws.queue.enqueue(a);

    WorkItem b;
    b.id = "b1"; b.nodeId = "n2"; b.nodeName = "f2"; b.nodeType = "Function";
    b.priority = "medium"; b.status = WI_READY;
    b.createdAt = workItemTimestamp();
    ws.queue.enqueue(b);

    CHECK(ws.getPhase() == WorkflowPhase::Executing, "some assigned = Executing");
    PASS();
}

// 5. Phase: some in review → Reviewing
void test_phase_reviewing() {
    TEST(phase_reviewing);
    WorkflowState ws("proj");

    WorkItem a;
    a.id = "r1"; a.nodeId = "n1"; a.nodeName = "f1"; a.nodeType = "Function";
    a.priority = "medium"; a.status = WI_REVIEW;
    a.createdAt = workItemTimestamp();
    ws.queue.enqueue(a);

    CHECK(ws.getPhase() == WorkflowPhase::Reviewing, "some in review = Reviewing");
    PASS();
}

// 6. Phase: all complete → Complete
void test_phase_complete() {
    TEST(phase_complete);
    WorkflowState ws("proj");

    WorkItem a;
    a.id = "c1"; a.nodeId = "n1"; a.nodeName = "f1"; a.nodeType = "Function";
    a.priority = "medium"; a.status = WI_COMPLETE;
    a.createdAt = workItemTimestamp();
    ws.queue.enqueue(a);

    WorkItem b;
    b.id = "c2"; b.nodeId = "n2"; b.nodeName = "f2"; b.nodeType = "Function";
    b.priority = "medium"; b.status = WI_COMPLETE;
    b.createdAt = workItemTimestamp();
    ws.queue.enqueue(b);

    CHECK(ws.getPhase() == WorkflowPhase::Complete, "all complete = Complete");
    PASS();
}

// 7. Stats accuracy
void test_stats_accuracy() {
    TEST(stats_accuracy);
    WorkflowState ws("proj");

    auto addItem = [&](const std::string& id, const std::string& status,
                       const std::string& wtype, const std::string& prio) {
        WorkItem wi;
        wi.id = id; wi.nodeId = "n_" + id; wi.nodeName = "f_" + id;
        wi.nodeType = "Function"; wi.priority = prio;
        wi.workerType = wtype; wi.status = status;
        wi.createdAt = workItemTimestamp();
        ws.queue.enqueue(wi);
    };

    addItem("s1", WI_READY, "llm", "high");
    addItem("s2", WI_READY, "llm", "medium");
    addItem("s3", WI_COMPLETE, "deterministic", "low");
    addItem("s4", WI_ASSIGNED, "llm", "high");

    auto stats = ws.getStats();
    CHECK(stats.total == 4, "total=4");
    CHECK(stats.ready == 2, "ready=2");
    CHECK(stats.complete == 1, "complete=1");
    CHECK(stats.assigned == 1, "assigned=1");
    CHECK(stats.byWorkerType["llm"] == 3, "llm=3");
    CHECK(stats.byWorkerType["deterministic"] == 1, "deterministic=1");
    CHECK(stats.byPriority["high"] == 2, "high=2");
    CHECK(stats.completionPercent > 24.0f && stats.completionPercent < 26.0f, "25%");
    PASS();
}

// 8. History tracking
void test_history_tracking() {
    TEST(history_tracking);
    WorkflowState ws("proj");
    ws.recordChange("item1", "", "pending", "system", "created");
    ws.recordChange("item1", "pending", "ready", "system", "deps met");
    ws.recordChange("item1", "ready", "assigned", "routing-engine", "routed to llm");

    auto hist = ws.getHistory("item1");
    CHECK(hist.size() == 3, "3 history entries");
    CHECK(hist[0].fromStatus == "", "first from empty");
    CHECK(hist[0].toStatus == "pending", "first to pending");
    CHECK(hist[2].actor == "routing-engine", "third actor");

    // Other item has no history
    auto hist2 = ws.getHistory("item2");
    CHECK(hist2.empty(), "no history for item2");
    PASS();
}

// 9. JSON roundtrip — full state preserved
void test_json_roundtrip() {
    TEST(json_roundtrip);
    WorkflowState ws("myproject");

    WorkItem a;
    a.id = "j1"; a.nodeId = "n1"; a.nodeName = "compute"; a.nodeType = "Function";
    a.priority = "high"; a.workerType = "llm"; a.status = WI_READY;
    a.createdAt = "2026-01-01T00:00:00Z";
    ws.queue.enqueue(a);

    WorkItem b;
    b.id = "j2"; b.nodeId = "n2"; b.nodeName = "validate"; b.nodeType = "Function";
    b.priority = "medium"; b.workerType = "deterministic"; b.status = WI_COMPLETE;
    b.createdAt = "2026-01-01T00:01:00Z";
    ws.queue.enqueue(b);

    ws.recordChange("j1", "", "ready", "system");
    ws.recordChange("j2", "", "complete", "worker:det-001");

    json j = ws.toJson();
    WorkflowState ws2 = WorkflowState::fromJson(j);

    CHECK(ws2.projectName == "myproject", "projectName");
    CHECK(ws2.queue.size() == 2, "2 items");
    CHECK(ws2.getHistory("j1").size() == 1, "j1 history preserved, got " +
          std::to_string(ws2.getHistory("j1").size()));
    CHECK(ws2.getHistory("j2").size() == 1, "j2 history preserved");

    auto item1 = ws2.queue.getItem("j1");
    CHECK(item1.has_value(), "j1 exists");
    CHECK(item1->nodeName == "compute", "j1 name");
    PASS();
}

// 10. Empty workflow behavior
void test_empty_workflow() {
    TEST(empty_workflow);
    WorkflowState ws("empty");
    auto stats = ws.getStats();
    CHECK(stats.total == 0, "total=0");
    CHECK(stats.completionPercent == 0.0f, "0% complete");
    CHECK(ws.getPhase() == WorkflowPhase::Modeling, "Modeling phase");
    CHECK(ws.getHistory("any").empty(), "no history");

    json j = ws.toJson();
    CHECK(j.contains("projectName"), "has projectName");
    CHECK(j["items"].size() == 0, "no items");
    PASS();
}

// 11. Phase transitions through lifecycle
void test_phase_transitions() {
    TEST(phase_transitions);
    WorkflowState ws("proj");

    // Start empty → Modeling
    CHECK(ws.getPhase() == WorkflowPhase::Modeling, "start=Modeling");

    // Add ready items → Routing
    WorkItem a;
    a.id = "pt1"; a.nodeId = "n1"; a.nodeName = "f1"; a.nodeType = "Function";
    a.priority = "medium"; a.status = WI_READY;
    a.createdAt = workItemTimestamp();
    ws.queue.enqueue(a);
    CHECK(ws.getPhase() == WorkflowPhase::Routing, "with ready=Routing");

    // Assign → Executing
    ws.queue.dequeue();
    CHECK(ws.getPhase() == WorkflowPhase::Executing, "with assigned=Executing");

    // Move to review
    WorkItem updated = *ws.queue.getItem("pt1");
    transitionWorkItem(updated, WI_IN_PROGRESS);
    ws.queue.updateItem("pt1", updated);
    updated = *ws.queue.getItem("pt1");
    transitionWorkItem(updated, WI_REVIEW);
    ws.queue.updateItem("pt1", updated);
    CHECK(ws.getPhase() == WorkflowPhase::Reviewing, "with review=Reviewing");

    // Complete
    ws.queue.complete("pt1");
    CHECK(ws.getPhase() == WorkflowPhase::Complete, "all complete=Complete");
    PASS();
}

// 12. populateFromSkeleton records history entries
void test_populate_records_history() {
    TEST(populate_records_history);
    auto* mod = makeSkeletonModule("proj2", 3);
    WorkflowState ws("proj2");
    ws.populateFromSkeleton(mod, "buf-2");

    // Each item should have a creation history entry
    int histCount = 0;
    for (const auto& status : {WI_PENDING, WI_READY}) {
        for (const auto& item : ws.queue.getByStatus(status)) {
            auto h = ws.getHistory(item.id);
            if (!h.empty()) histCount++;
        }
    }
    CHECK(histCount == 3, "3 items have history, got " + std::to_string(histCount));
    delete mod;
    PASS();
}

int main() {
    std::cout << "=== Step 322: WorkflowState Project-Level Workflow Tracking ===\n";
    try {
        test_populate_from_skeleton();
        test_phase_empty();
        test_phase_all_pending();
        test_phase_executing();
        test_phase_reviewing();
        test_phase_complete();
        test_stats_accuracy();
        test_history_tracking();
        test_json_roundtrip();
        test_empty_workflow();
        test_phase_transitions();
        test_populate_records_history();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
