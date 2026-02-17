// Step 564: Memory Snapshot Model (12 tests)

#include "MemorySnapshotModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_create_snapshot_success() {
    TEST(create_snapshot_success);
    MemorySnapshot snapshot;
    std::string error;
    const bool ok = MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 7, &snapshot, &error);
    CHECK(ok, "snapshot creation should succeed");
    CHECK(snapshot.snapshotId == "snap-1", "snapshot id mismatch");
    CHECK(snapshot.sessionId == "sess-1", "session id mismatch");
    CHECK(snapshot.threadId == "thread-1", "thread id mismatch");
    CHECK(snapshot.sequence == 7, "sequence mismatch");
    PASS();
}

void test_create_snapshot_fails_with_missing_ids() {
    TEST(create_snapshot_fails_with_missing_ids);
    MemorySnapshot snapshot;
    std::string error;
    const bool ok = MemorySnapshotModel::createSnapshot("", "sess-1", "thread-1", 1, &snapshot, &error);
    CHECK(!ok, "creation should fail with missing snapshot id");
    CHECK(error == "snapshot_id_or_session_id_missing", "wrong error");
    PASS();
}

void test_create_snapshot_fails_with_missing_thread() {
    TEST(create_snapshot_fails_with_missing_thread);
    MemorySnapshot snapshot;
    std::string error;
    const bool ok = MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "", 1, &snapshot, &error);
    CHECK(!ok, "creation should fail with missing thread id");
    CHECK(error == "thread_id_missing", "wrong error");
    PASS();
}

void test_add_region_success() {
    TEST(add_region_success);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    MemoryRegionRecord stackRegion{"r-stack", MemoryRegionKind::Stack, 0x1000, 64, false};
    CHECK(MemorySnapshotModel::addRegion(&snapshot, stackRegion, &error), "add region failed");
    CHECK(snapshot.regions.size() == 1, "region count should be one");
    CHECK(snapshot.regions[0].id == "r-stack", "region id mismatch");
    PASS();
}

void test_add_region_rejects_duplicate_id() {
    TEST(add_region_rejects_duplicate_id);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    MemoryRegionRecord first{"dup", MemoryRegionKind::Heap, 0x2000, 32, false};
    MemoryRegionRecord second{"dup", MemoryRegionKind::Heap, 0x3000, 32, false};
    CHECK(MemorySnapshotModel::addRegion(&snapshot, first, &error), "first add failed");
    CHECK(!MemorySnapshotModel::addRegion(&snapshot, second, &error), "duplicate id should fail");
    CHECK(error == "region_id_duplicate", "wrong duplicate error");
    PASS();
}

void test_add_region_rejects_invalid_bounds() {
    TEST(add_region_rejects_invalid_bounds);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    MemoryRegionRecord badStart{"bad-start", MemoryRegionKind::Heap, 0, 32, false};
    CHECK(!MemorySnapshotModel::addRegion(&snapshot, badStart, &error), "zero start should fail");
    CHECK(error == "region_start_address_invalid", "wrong start-address error");
    MemoryRegionRecord badSize{"bad-size", MemoryRegionKind::Heap, 0x2000, 0, false};
    CHECK(!MemorySnapshotModel::addRegion(&snapshot, badSize, &error), "zero size should fail");
    CHECK(error == "region_size_invalid", "wrong size error");
    PASS();
}

void test_regions_are_sorted_by_start_address() {
    TEST(regions_are_sorted_by_start_address);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    CHECK(MemorySnapshotModel::addRegion(&snapshot, {"r2", MemoryRegionKind::Heap, 0x3000, 16, false}, &error), "add r2 failed");
    CHECK(MemorySnapshotModel::addRegion(&snapshot, {"r1", MemoryRegionKind::Stack, 0x1000, 16, false}, &error), "add r1 failed");
    CHECK(MemorySnapshotModel::addRegion(&snapshot, {"r3", MemoryRegionKind::Global, 0x2000, 16, true}, &error), "add r3 failed");
    CHECK(snapshot.regions[0].id == "r1", "first region should be r1");
    CHECK(snapshot.regions[1].id == "r3", "second region should be r3");
    CHECK(snapshot.regions[2].id == "r2", "third region should be r2");
    PASS();
}

void test_find_region_by_address_within_bounds() {
    TEST(find_region_by_address_within_bounds);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    CHECK(MemorySnapshotModel::addRegion(&snapshot, {"heap-a", MemoryRegionKind::Heap, 0x5000, 128, false}, &error), "add failed");
    const MemoryRegionRecord* found = MemorySnapshotModel::findRegionByAddress(snapshot, 0x507F);
    CHECK(found != nullptr, "region should be found");
    CHECK(found->id == "heap-a", "wrong region resolved");
    PASS();
}

void test_find_region_by_address_at_exclusive_end_returns_null() {
    TEST(find_region_by_address_at_exclusive_end_returns_null);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    CHECK(MemorySnapshotModel::addRegion(&snapshot, {"heap-a", MemoryRegionKind::Heap, 0x5000, 128, false}, &error), "add failed");
    const MemoryRegionRecord* found = MemorySnapshotModel::findRegionByAddress(snapshot, 0x5080);
    CHECK(found == nullptr, "exclusive upper-bound should not match");
    PASS();
}

void test_add_reference_success() {
    TEST(add_reference_success);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    MemoryReferenceRecord ref{0x4000, 0x7000, "next"};
    CHECK(MemorySnapshotModel::addReference(&snapshot, ref, &error), "add reference failed");
    CHECK(snapshot.references.size() == 1, "reference count should be one");
    CHECK(snapshot.references[0].label == "next", "label mismatch");
    PASS();
}

void test_add_reference_rejects_invalid_fields() {
    TEST(add_reference_rejects_invalid_fields);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    CHECK(!MemorySnapshotModel::addReference(&snapshot, {0, 0x7000, "next"}, &error), "zero source should fail");
    CHECK(error == "reference_address_invalid", "wrong address error");
    CHECK(!MemorySnapshotModel::addReference(&snapshot, {0x4000, 0x7000, ""}, &error), "empty label should fail");
    CHECK(error == "reference_label_missing", "wrong label error");
    PASS();
}

void test_outgoing_references_filter_by_source_address() {
    TEST(outgoing_references_filter_by_source_address);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap-1", "sess-1", "thread-1", 1, &snapshot, &error), "create failed");
    CHECK(MemorySnapshotModel::addReference(&snapshot, {0x1000, 0x2000, "a"}, &error), "add ref a failed");
    CHECK(MemorySnapshotModel::addReference(&snapshot, {0x1000, 0x3000, "b"}, &error), "add ref b failed");
    CHECK(MemorySnapshotModel::addReference(&snapshot, {0x5000, 0x6000, "c"}, &error), "add ref c failed");
    const auto refs = MemorySnapshotModel::outgoingReferences(snapshot, 0x1000);
    CHECK(refs.size() == 2, "expected two outgoing refs");
    CHECK(refs[0].label == "a", "first outgoing label mismatch");
    CHECK(refs[1].label == "b", "second outgoing label mismatch");
    PASS();
}

int main() {
    std::cout << "Step 564: Memory Snapshot Model\n";

    test_create_snapshot_success();                                // 1
    test_create_snapshot_fails_with_missing_ids();                 // 2
    test_create_snapshot_fails_with_missing_thread();              // 3
    test_add_region_success();                                     // 4
    test_add_region_rejects_duplicate_id();                        // 5
    test_add_region_rejects_invalid_bounds();                      // 6
    test_regions_are_sorted_by_start_address();                    // 7
    test_find_region_by_address_within_bounds();                   // 8
    test_find_region_by_address_at_exclusive_end_returns_null();   // 9
    test_add_reference_success();                                  // 10
    test_add_reference_rejects_invalid_fields();                   // 11
    test_outgoing_references_filter_by_source_address();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
