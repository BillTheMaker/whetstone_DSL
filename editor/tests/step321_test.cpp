// Step 321: TaskQueue — Priority Queue with Dependencies (12 tests)
// Tests enqueue/dequeue ordering, dependency blocking, reject/re-enqueue,
// size tracking, completion cascading, mixed priorities with dependencies.

#include "TaskQueue.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: create a WorkItem with given id, priority, and optional dependencies
static WorkItem makeItem(const std::string& id, const std::string& priority,
                         const std::vector<std::string>& deps = {}) {
    WorkItem wi;
    wi.id = id;
    wi.nodeId = "node_" + id;
    wi.nodeName = "func_" + id;
    wi.nodeType = "Function";
    wi.bufferId = "buf";
    wi.contextWidth = "local";
    wi.workerType = "llm";
    wi.priority = priority;
    wi.dependencies = deps;
    wi.status = WI_PENDING;
    wi.createdAt = workItemTimestamp();
    return wi;
}

// 1. Enqueue/dequeue basic ordering
void test_enqueue_dequeue() {
    TEST(enqueue_dequeue);
    TaskQueue q;
    q.enqueue(makeItem("a", "medium"));
    q.enqueue(makeItem("b", "medium"));

    CHECK(q.size() == 2, "size=2");
    auto item = q.dequeue();
    CHECK(item.has_value(), "dequeue returns item");
    CHECK(item->id == "a", "FIFO for same priority: got " + item->id);
    PASS();
}

// 2. Priority ordering — critical before low
void test_priority_ordering() {
    TEST(priority_ordering);
    TaskQueue q;
    q.enqueue(makeItem("low1", "low"));
    q.enqueue(makeItem("crit1", "critical"));
    q.enqueue(makeItem("med1", "medium"));
    q.enqueue(makeItem("high1", "high"));

    auto ready = q.getReady();
    CHECK(ready.size() == 4, "4 ready");
    CHECK(ready[0].id == "crit1", "first=critical");
    CHECK(ready[1].id == "high1", "second=high");
    CHECK(ready[2].id == "med1", "third=medium");
    CHECK(ready[3].id == "low1", "fourth=low");
    PASS();
}

// 3. Dependency blocking — B blocked by A, A completes → B becomes ready
void test_dependency_blocking() {
    TEST(dependency_blocking);
    TaskQueue q;
    q.enqueue(makeItem("A", "medium"));
    q.enqueue(makeItem("B", "medium", {"A"}));

    CHECK(q.readyCount() == 1, "1 ready (A)");
    CHECK(q.blockedCount() == 1, "1 blocked (B)");

    // Dequeue A, progress it through to complete
    auto a = q.dequeue();
    CHECK(a.has_value(), "dequeued A");
    // Manually transition: assigned → in-progress → complete
    auto aItem = q.getItem("A");
    CHECK(aItem.has_value(), "A still in queue");
    WorkItem updated = *aItem;
    transitionWorkItem(updated, WI_IN_PROGRESS);
    q.updateItem("A", updated);
    q.complete("A");

    CHECK(q.readyCount() == 1, "B now ready");
    CHECK(q.blockedCount() == 0, "0 blocked");
    auto b = q.peek();
    CHECK(b.has_value() && b->id == "B", "B is next");
    PASS();
}

// 4. Reject and re-enqueue
void test_reject_reenqueue() {
    TEST(reject_reenqueue);
    TaskQueue q;
    q.enqueue(makeItem("X", "high"));

    auto x = q.dequeue();
    CHECK(x.has_value(), "dequeued X");

    // Move to in-progress → review
    WorkItem updated = *q.getItem("X");
    transitionWorkItem(updated, WI_IN_PROGRESS);
    q.updateItem("X", updated);
    updated = *q.getItem("X");
    transitionWorkItem(updated, WI_REVIEW);
    q.updateItem("X", updated);

    bool rejected = q.reject("X", "needs better error handling");
    CHECK(rejected, "reject succeeded");

    auto xNow = q.getItem("X");
    CHECK(xNow.has_value(), "X still exists");
    CHECK(xNow->status == WI_READY, "X is ready again");
    CHECK(xNow->result.reasoning == "needs better error handling", "feedback preserved");
    PASS();
}

// 5. getReady returns only unblocked items
void test_get_ready_filtering() {
    TEST(get_ready_filtering);
    TaskQueue q;
    q.enqueue(makeItem("r1", "medium"));
    q.enqueue(makeItem("r2", "medium"));
    q.enqueue(makeItem("b1", "medium", {"r1"}));
    q.enqueue(makeItem("b2", "medium", {"r2"}));

    auto ready = q.getReady();
    CHECK(ready.size() == 2, "2 ready, got " + std::to_string(ready.size()));
    // blocked items should not appear
    for (const auto& r : ready) {
        CHECK(r.id == "r1" || r.id == "r2", "only r1/r2 in ready");
    }
    PASS();
}

// 6. Empty queue behavior
void test_empty_queue() {
    TEST(empty_queue);
    TaskQueue q;
    CHECK(q.size() == 0, "empty size");
    CHECK(q.readyCount() == 0, "empty readyCount");
    CHECK(q.blockedCount() == 0, "empty blockedCount");
    CHECK(!q.peek().has_value(), "peek returns nullopt");
    CHECK(!q.dequeue().has_value(), "dequeue returns nullopt");
    CHECK(!q.getItem("nonexistent").has_value(), "getItem returns nullopt");
    PASS();
}

// 7. getByStatus filtering
void test_get_by_status() {
    TEST(get_by_status);
    TaskQueue q;
    q.enqueue(makeItem("s1", "medium"));
    q.enqueue(makeItem("s2", "medium"));
    q.enqueue(makeItem("s3", "medium", {"s1"}));

    auto ready = q.getByStatus(WI_READY);
    CHECK(ready.size() == 2, "2 ready");
    auto pending = q.getByStatus(WI_PENDING);
    CHECK(pending.size() == 1, "1 pending");

    q.dequeue(); // assign s1
    auto assigned = q.getByStatus(WI_ASSIGNED);
    CHECK(assigned.size() == 1, "1 assigned");
    PASS();
}

// 8. Size tracking after operations
void test_size_tracking() {
    TEST(size_tracking);
    TaskQueue q;
    q.enqueue(makeItem("t1", "low"));
    q.enqueue(makeItem("t2", "high"));
    q.enqueue(makeItem("t3", "medium", {"t1"}));

    CHECK(q.size() == 3, "size=3");
    CHECK(q.readyCount() == 2, "readyCount=2");
    CHECK(q.blockedCount() == 1, "blockedCount=1");

    q.dequeue(); // dequeue t2 (highest priority)
    CHECK(q.readyCount() == 1, "readyCount=1 after dequeue");
    PASS();
}

// 9. Completion cascading (A → B → C chain)
void test_completion_cascade() {
    TEST(completion_cascade);
    TaskQueue q;
    q.enqueue(makeItem("c1", "high"));
    q.enqueue(makeItem("c2", "high", {"c1"}));
    q.enqueue(makeItem("c3", "high", {"c2"}));

    CHECK(q.readyCount() == 1, "only c1 ready");
    CHECK(q.blockedCount() == 2, "c2, c3 blocked");

    // Complete c1
    q.dequeue(); // assign c1
    WorkItem u = *q.getItem("c1");
    transitionWorkItem(u, WI_IN_PROGRESS);
    q.updateItem("c1", u);
    q.complete("c1");

    CHECK(q.readyCount() == 1, "c2 now ready");
    CHECK(q.blockedCount() == 1, "c3 still blocked");

    // Complete c2
    q.dequeue(); // assign c2
    u = *q.getItem("c2");
    transitionWorkItem(u, WI_IN_PROGRESS);
    q.updateItem("c2", u);
    q.complete("c2");

    CHECK(q.readyCount() == 1, "c3 now ready");
    CHECK(q.blockedCount() == 0, "none blocked");
    PASS();
}

// 10. Mixed priorities with dependencies
void test_mixed_priorities_deps() {
    TEST(mixed_priorities_deps);
    TaskQueue q;
    q.enqueue(makeItem("lo", "low"));
    q.enqueue(makeItem("hi", "critical", {"lo"}));

    // hi is critical but blocked, lo is low but ready
    auto ready = q.getReady();
    CHECK(ready.size() == 1, "1 ready");
    CHECK(ready[0].id == "lo", "lo is ready despite lower priority");

    // Complete lo → hi becomes ready
    q.dequeue();
    WorkItem u = *q.getItem("lo");
    transitionWorkItem(u, WI_IN_PROGRESS);
    q.updateItem("lo", u);
    q.complete("lo");

    ready = q.getReady();
    CHECK(ready.size() == 1, "hi now ready");
    CHECK(ready[0].id == "hi", "hi unblocked");
    PASS();
}

// 11. Peek does not remove
void test_peek_no_remove() {
    TEST(peek_no_remove);
    TaskQueue q;
    q.enqueue(makeItem("p1", "medium"));

    auto first = q.peek();
    CHECK(first.has_value(), "peek returns value");
    auto second = q.peek();
    CHECK(second.has_value(), "peek still returns value");
    CHECK(first->id == second->id, "same item");
    CHECK(q.readyCount() == 1, "still 1 ready");
    PASS();
}

// 12. updateItem replaces item data
void test_update_item() {
    TEST(update_item);
    TaskQueue q;
    q.enqueue(makeItem("u1", "medium"));

    auto item = *q.getItem("u1");
    item.assignee = "worker-42";
    item.workerType = "deterministic";
    q.updateItem("u1", item);

    auto updated = q.getItem("u1");
    CHECK(updated.has_value(), "item exists");
    CHECK(updated->assignee == "worker-42", "assignee updated");
    CHECK(updated->workerType == "deterministic", "workerType updated");
    PASS();
}

int main() {
    std::cout << "=== Step 321: TaskQueue Priority Queue with Dependencies ===\n";
    try {
        test_enqueue_dequeue();
        test_priority_ordering();
        test_dependency_blocking();
        test_reject_reenqueue();
        test_get_ready_filtering();
        test_empty_queue();
        test_get_by_status();
        test_size_tracking();
        test_completion_cascade();
        test_mixed_priorities_deps();
        test_peek_no_remove();
        test_update_item();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
