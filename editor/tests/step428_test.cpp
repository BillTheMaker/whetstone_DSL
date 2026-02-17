// Step 428: Kanban-Style Task Board Tests (12 tests)

#include "WorkflowTaskBoard.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& id, const std::string& status,
                         const std::string& worker, const std::string& priority,
                         const std::string& file, const std::string& name) {
    WorkItem wi;
    wi.id = id;
    wi.status = status;
    wi.workerType = worker;
    wi.priority = priority;
    wi.bufferId = file;
    wi.nodeName = name;
    wi.nodeId = id + "_node";
    wi.contextWidth = "file";
    wi.reviewRequired = (status == WI_REVIEW);
    if (status == WI_PENDING) wi.dependencies = {"dep-blocker"};
    return wi;
}

static WorkflowState makeWorkflow() {
    WorkflowState wf("task-board");
    wf.queue.enqueue(makeItem("w1", WI_PENDING, "llm", "high", "src/a.cpp", "TaskA"));
    wf.queue.enqueue(makeItem("w2", WI_READY, "slm", "medium", "src/b.py", "TaskB"));
    wf.queue.enqueue(makeItem("w3", WI_IN_PROGRESS, "template", "low", "src/c.cpp", "TaskC"));
    wf.queue.enqueue(makeItem("w4", WI_REVIEW, "llm", "critical", "src/d.ts", "TaskD"));
    wf.queue.enqueue(makeItem("w5", WI_COMPLETE, "human", "high", "src/e.cpp", "TaskE"));
    return wf;
}

static int countCards(const std::vector<TaskBoardColumn>& cols, const std::string& key) {
    for (const auto& c : cols) if (c.key == key) return (int)c.cards.size();
    return 0;
}

void test_board_has_five_columns() {
    TEST(board_has_five_columns);
    auto cols = WorkflowTaskBoard::build(makeWorkflow());
    CHECK(cols.size() == 5, "expected 5 columns");
    PASS();
}

void test_statuses_map_to_expected_columns() {
    TEST(statuses_map_to_expected_columns);
    auto cols = WorkflowTaskBoard::build(makeWorkflow());
    CHECK(countCards(cols, "pending") >= 1, "pending column should contain at least one card");
    CHECK(countCards(cols, "ready") == 1, "ready count mismatch");
    CHECK(countCards(cols, "in-progress") == 1, "in-progress count mismatch");
    CHECK(countCards(cols, "review") == 1, "review count mismatch");
    CHECK(countCards(cols, "complete") == 1, "complete count mismatch");
    PASS();
}

void test_assigned_maps_to_in_progress_column() {
    TEST(assigned_maps_to_in_progress_column);
    WorkflowState wf("x");
    wf.queue.enqueue(makeItem("w1", WI_ASSIGNED, "llm", "high", "f.cpp", "TaskA"));
    auto cols = WorkflowTaskBoard::build(wf);
    CHECK(countCards(cols, "in-progress") == 1, "assigned should map to in-progress");
    PASS();
}

void test_filter_by_worker_type() {
    TEST(filter_by_worker_type);
    TaskBoardFilter f;
    f.workerType = "llm";
    auto cols = WorkflowTaskBoard::build(makeWorkflow(), f);
    CHECK(countCards(cols, "pending") >= 1, "llm pending missing");
    CHECK(countCards(cols, "review") == 1, "llm review missing");
    CHECK(countCards(cols, "ready") == 0, "non-llm should be filtered out");
    PASS();
}

void test_filter_by_priority() {
    TEST(filter_by_priority);
    TaskBoardFilter f;
    f.priority = "critical";
    auto cols = WorkflowTaskBoard::build(makeWorkflow(), f);
    CHECK(countCards(cols, "review") == 1, "critical review missing");
    CHECK(countCards(cols, "pending") == 0, "non-critical item should be filtered");
    PASS();
}

void test_filter_by_language() {
    TEST(filter_by_language);
    TaskBoardFilter f;
    f.language = "python";
    auto cols = WorkflowTaskBoard::build(makeWorkflow(), f);
    CHECK(countCards(cols, "ready") == 1, "python card missing");
    CHECK(countCards(cols, "pending") == 0, "cpp should be filtered");
    PASS();
}

void test_filter_by_file_substring() {
    TEST(filter_by_file_substring);
    TaskBoardFilter f;
    f.fileContains = "d.ts";
    auto cols = WorkflowTaskBoard::build(makeWorkflow(), f);
    CHECK(countCards(cols, "review") == 1, "expected d.ts task in review");
    CHECK(countCards(cols, "pending") == 0, "other files should be filtered");
    PASS();
}

void test_card_contains_worker_icon_and_tags() {
    TEST(card_contains_worker_icon_and_tags);
    auto cols = WorkflowTaskBoard::build(makeWorkflow());
    TaskCard reviewCard;
    bool found = false;
    for (const auto& c : cols) {
        if (c.key != "review" || c.cards.empty()) continue;
        reviewCard = c.cards[0];
        found = true;
        break;
    }
    CHECK(found, "review card missing");
    CHECK(reviewCard.workerIcon == "L", "llm icon should be L");
    CHECK(!reviewCard.tags.empty(), "tags should not be empty");
    PASS();
}

void test_move_item_between_columns() {
    TEST(move_item_between_columns);
    auto wf = makeWorkflow();
    CHECK(WorkflowTaskBoard::moveItem(wf, "w1", "ready"), "move should succeed");
    auto moved = wf.queue.getItem("w1");
    CHECK(moved.has_value(), "item missing after move");
    CHECK(moved->status == WI_READY, "status should be ready");
    PASS();
}

void test_move_item_invalid_column_fails() {
    TEST(move_item_invalid_column_fails);
    auto wf = makeWorkflow();
    CHECK(!WorkflowTaskBoard::moveItem(wf, "w1", "unknown"), "invalid column should fail");
    PASS();
}

void test_move_item_missing_item_fails() {
    TEST(move_item_missing_item_fails);
    auto wf = makeWorkflow();
    CHECK(!WorkflowTaskBoard::moveItem(wf, "missing", "ready"), "missing item should fail");
    PASS();
}

void test_cards_sorted_by_priority_then_id() {
    TEST(cards_sorted_by_priority_then_id);
    WorkflowState wf("sort");
    wf.queue.enqueue(makeItem("w2", WI_READY, "llm", "medium", "f.cpp", "B"));
    wf.queue.enqueue(makeItem("w1", WI_READY, "llm", "high", "f.cpp", "A"));
    auto cols = WorkflowTaskBoard::build(wf);
    TaskBoardColumn ready;
    for (const auto& c : cols) if (c.key == "ready") ready = c;
    CHECK(ready.cards.size() == 2, "ready should have two cards");
    CHECK(ready.cards[0].itemId == "w1", "high priority should sort first");
    PASS();
}

int main() {
    std::cout << "Step 428: Kanban-Style Task Board Tests\n";

    test_board_has_five_columns();             // 1
    test_statuses_map_to_expected_columns();   // 2
    test_assigned_maps_to_in_progress_column();// 3
    test_filter_by_worker_type();              // 4
    test_filter_by_priority();                 // 5
    test_filter_by_language();                 // 6
    test_filter_by_file_substring();           // 7
    test_card_contains_worker_icon_and_tags(); // 8
    test_move_item_between_columns();          // 9
    test_move_item_invalid_column_fails();     // 10
    test_move_item_missing_item_fails();       // 11
    test_cards_sorted_by_priority_then_id();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
