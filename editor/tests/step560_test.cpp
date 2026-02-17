// Step 560: Locals + Watches Panel Data Model (12 tests)

#include "LocalsWatchesPanelModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_set_locals_persists_entries() {
    TEST(set_locals_persists_entries);
    LocalsWatchesPanelModel m;
    m.setLocals({{"cursor", "int", "12"}, {"state", "std::string", "ok"}});
    CHECK(m.locals().size() == 2, "two locals expected");
    PASS();
}

void test_locals_preserve_type_information() {
    TEST(locals_preserve_type_information);
    LocalsWatchesPanelModel m;
    m.setLocals({{"cursor", "int", "12"}});
    CHECK(m.locals()[0].type == "int", "type info mismatch");
    PASS();
}

void test_add_watch_succeeds_for_unique_id() {
    TEST(add_watch_succeeds_for_unique_id);
    LocalsWatchesPanelModel m;
    CHECK(m.addWatch("w1", "cursor"), "add watch should succeed");
    CHECK(m.watches().size() == 1, "one watch expected");
    PASS();
}

void test_add_watch_rejects_duplicate_id() {
    TEST(add_watch_rejects_duplicate_id);
    LocalsWatchesPanelModel m;
    CHECK(m.addWatch("w1", "cursor"), "first add should succeed");
    CHECK(!m.addWatch("w1", "state"), "duplicate id should fail");
    PASS();
}

void test_add_watch_rejects_empty_fields() {
    TEST(add_watch_rejects_empty_fields);
    LocalsWatchesPanelModel m;
    CHECK(!m.addWatch("", "cursor"), "empty id should fail");
    CHECK(!m.addWatch("w1", ""), "empty expression should fail");
    PASS();
}

void test_remove_watch_succeeds() {
    TEST(remove_watch_succeeds);
    LocalsWatchesPanelModel m;
    CHECK(m.addWatch("w1", "cursor"), "add should succeed");
    CHECK(m.removeWatch("w1"), "remove should succeed");
    CHECK(m.watches().empty(), "watches should be empty");
    PASS();
}

void test_remove_watch_fails_for_unknown_id() {
    TEST(remove_watch_fails_for_unknown_id);
    LocalsWatchesPanelModel m;
    CHECK(!m.removeWatch("missing"), "remove unknown should fail");
    PASS();
}

void test_enable_disable_watch() {
    TEST(enable_disable_watch);
    LocalsWatchesPanelModel m;
    CHECK(m.addWatch("w1", "cursor"), "add should succeed");
    CHECK(m.enableWatch("w1", false), "disable should succeed");
    CHECK(!m.watches()[0].enabled, "watch should be disabled");
    CHECK(m.enableWatch("w1", true), "enable should succeed");
    CHECK(m.watches()[0].enabled, "watch should be enabled");
    PASS();
}

void test_update_watch_value_tracks_eval_status() {
    TEST(update_watch_value_tracks_eval_status);
    LocalsWatchesPanelModel m;
    CHECK(m.addWatch("w1", "cursor"), "add should succeed");
    CHECK(m.updateWatchValue("w1", "12", true), "update should succeed");
    CHECK(m.watches()[0].lastValue == "12", "value mismatch");
    CHECK(m.watches()[0].lastEvalOk, "eval status mismatch");
    PASS();
}

void test_update_watch_value_fails_for_unknown_watch() {
    TEST(update_watch_value_fails_for_unknown_watch);
    LocalsWatchesPanelModel m;
    CHECK(!m.updateWatchValue("missing", "x", true), "unknown watch update should fail");
    PASS();
}

void test_active_watches_filters_disabled_entries() {
    TEST(active_watches_filters_disabled_entries);
    LocalsWatchesPanelModel m;
    CHECK(m.addWatch("w1", "cursor"), "add w1");
    CHECK(m.addWatch("w2", "state"), "add w2");
    CHECK(m.enableWatch("w2", false), "disable w2");
    auto active = m.activeWatches();
    CHECK(active.size() == 1, "only one active watch expected");
    CHECK(active[0].id == "w1", "active watch mismatch");
    PASS();
}

void test_remove_rebuilds_index_for_remaining_watches() {
    TEST(remove_rebuilds_index_for_remaining_watches);
    LocalsWatchesPanelModel m;
    CHECK(m.addWatch("w1", "cursor"), "add w1");
    CHECK(m.addWatch("w2", "state"), "add w2");
    CHECK(m.removeWatch("w1"), "remove w1");
    CHECK(m.updateWatchValue("w2", "ok", true), "w2 should remain addressable");
    CHECK(m.watches()[0].id == "w2", "remaining watch should be w2");
    PASS();
}

int main() {
    std::cout << "Step 560: Locals + Watches Panel Data Model\n";

    test_set_locals_persists_entries();                   // 1
    test_locals_preserve_type_information();              // 2
    test_add_watch_succeeds_for_unique_id();              // 3
    test_add_watch_rejects_duplicate_id();                // 4
    test_add_watch_rejects_empty_fields();                // 5
    test_remove_watch_succeeds();                         // 6
    test_remove_watch_fails_for_unknown_id();             // 7
    test_enable_disable_watch();                          // 8
    test_update_watch_value_tracks_eval_status();         // 9
    test_update_watch_value_fails_for_unknown_watch();    // 10
    test_active_watches_filters_disabled_entries();       // 11
    test_remove_rebuilds_index_for_remaining_watches();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
