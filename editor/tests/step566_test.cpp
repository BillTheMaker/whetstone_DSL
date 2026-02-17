// Step 566: Allocation/Ownership Trace Hooks (12 tests)

#include "AllocationOwnershipTraceHooks.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_record_allocation_success() {
    TEST(record_allocation_success);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    const bool ok = hooks.recordAllocation("sess-1", "alloc-1", 0x1000, 64, "owner-a", 10, &error);
    CHECK(ok, "allocation record should succeed");
    CHECK(hooks.activeAllocationCount() == 1, "active count should be one");
    CHECK(hooks.ownerFor("alloc-1") == "owner-a", "owner mismatch");
    PASS();
}

void test_record_allocation_rejects_missing_ids() {
    TEST(record_allocation_rejects_missing_ids);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    const bool ok = hooks.recordAllocation("", "alloc-1", 0x1000, 64, "owner-a", 10, &error);
    CHECK(!ok, "allocation record should fail");
    CHECK(error == "allocation_or_session_missing", "wrong error");
    PASS();
}

void test_record_allocation_rejects_invalid_address() {
    TEST(record_allocation_rejects_invalid_address);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    const bool ok = hooks.recordAllocation("sess-1", "alloc-1", 0, 64, "owner-a", 10, &error);
    CHECK(!ok, "zero address should fail");
    CHECK(error == "allocation_address_invalid", "wrong error");
    PASS();
}

void test_record_allocation_rejects_invalid_size() {
    TEST(record_allocation_rejects_invalid_size);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    const bool ok = hooks.recordAllocation("sess-1", "alloc-1", 0x1000, 0, "owner-a", 10, &error);
    CHECK(!ok, "zero size should fail");
    CHECK(error == "allocation_size_invalid", "wrong error");
    PASS();
}

void test_record_allocation_rejects_active_duplicate() {
    TEST(record_allocation_rejects_active_duplicate);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    CHECK(hooks.recordAllocation("sess-1", "alloc-1", 0x1000, 64, "owner-a", 10, &error), "first allocation failed");
    CHECK(!hooks.recordAllocation("sess-1", "alloc-1", 0x2000, 32, "owner-b", 11, &error), "duplicate should fail");
    CHECK(error == "allocation_id_active_duplicate", "wrong error");
    PASS();
}

void test_transfer_ownership_success() {
    TEST(transfer_ownership_success);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    CHECK(hooks.recordAllocation("sess-1", "alloc-1", 0x1000, 64, "owner-a", 10, &error), "allocation failed");
    CHECK(hooks.transferOwnership("sess-1", "alloc-1", "owner-b", 20, &error), "transfer failed");
    CHECK(hooks.ownerFor("alloc-1") == "owner-b", "owner should change");
    PASS();
}

void test_transfer_ownership_rejects_unknown_allocation() {
    TEST(transfer_ownership_rejects_unknown_allocation);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    const bool ok = hooks.transferOwnership("sess-1", "alloc-404", "owner-b", 20, &error);
    CHECK(!ok, "transfer should fail");
    CHECK(error == "allocation_not_active", "wrong error");
    PASS();
}

void test_transfer_ownership_rejects_unchanged_owner() {
    TEST(transfer_ownership_rejects_unchanged_owner);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    CHECK(hooks.recordAllocation("sess-1", "alloc-1", 0x1000, 64, "owner-a", 10, &error), "allocation failed");
    CHECK(!hooks.transferOwnership("sess-1", "alloc-1", "owner-a", 20, &error), "unchanged transfer should fail");
    CHECK(error == "transfer_owner_unchanged", "wrong error");
    PASS();
}

void test_record_free_success() {
    TEST(record_free_success);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    CHECK(hooks.recordAllocation("sess-1", "alloc-1", 0x1000, 64, "owner-a", 10, &error), "allocation failed");
    CHECK(hooks.recordFree("sess-1", "alloc-1", 30, &error), "free should succeed");
    CHECK(hooks.ownerFor("alloc-1").empty(), "owner should be cleared");
    CHECK(hooks.activeAllocationCount() == 0, "active count should be zero");
    PASS();
}

void test_record_free_rejects_unknown_allocation() {
    TEST(record_free_rejects_unknown_allocation);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    const bool ok = hooks.recordFree("sess-1", "alloc-x", 30, &error);
    CHECK(!ok, "free should fail");
    CHECK(error == "allocation_not_active", "wrong error");
    PASS();
}

void test_timeline_orders_by_timestamp() {
    TEST(timeline_orders_by_timestamp);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    CHECK(hooks.recordAllocation("sess-1", "alloc-a", 0x1000, 64, "a", 30, &error), "alloc-a failed");
    CHECK(hooks.recordAllocation("sess-1", "alloc-b", 0x2000, 64, "b", 10, &error), "alloc-b failed");
    CHECK(hooks.transferOwnership("sess-1", "alloc-b", "c", 20, &error), "transfer failed");
    const auto timeline = hooks.timelineForSession("sess-1");
    CHECK(timeline.size() == 3, "timeline count mismatch");
    CHECK(timeline[0].allocationId == "alloc-b", "first event should be alloc-b");
    CHECK(timeline[1].type == AllocationTraceEventType::Transfer, "second should be transfer");
    CHECK(timeline[2].allocationId == "alloc-a", "third should be alloc-a");
    PASS();
}

void test_timeline_for_session_filters_events() {
    TEST(timeline_for_session_filters_events);
    AllocationOwnershipTraceHooks hooks;
    std::string error;
    CHECK(hooks.recordAllocation("sess-1", "alloc-a", 0x1000, 64, "a", 10, &error), "sess-1 alloc failed");
    CHECK(hooks.recordAllocation("sess-2", "alloc-b", 0x2000, 64, "b", 11, &error), "sess-2 alloc failed");
    const auto timeline = hooks.timelineForSession("sess-2");
    CHECK(timeline.size() == 1, "session filter should return one event");
    CHECK(timeline[0].sessionId == "sess-2", "wrong session");
    PASS();
}

int main() {
    std::cout << "Step 566: Allocation/Ownership Trace Hooks\n";

    test_record_allocation_success();                     // 1
    test_record_allocation_rejects_missing_ids();         // 2
    test_record_allocation_rejects_invalid_address();     // 3
    test_record_allocation_rejects_invalid_size();        // 4
    test_record_allocation_rejects_active_duplicate();    // 5
    test_transfer_ownership_success();                    // 6
    test_transfer_ownership_rejects_unknown_allocation(); // 7
    test_transfer_ownership_rejects_unchanged_owner();    // 8
    test_record_free_success();                           // 9
    test_record_free_rejects_unknown_allocation();        // 10
    test_timeline_orders_by_timestamp();                  // 11
    test_timeline_for_session_filters_events();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
