// Step 565: Memory Inspector UI Model (12 tests)

#include "MemoryInspectorUiModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_create_leaf_success() {
    TEST(create_leaf_success);
    MemoryValueNode n;
    std::string error;
    const bool ok = MemoryInspectorUiModel::createLeaf("n1", "counter", MemoryValueKind::Integer, "42", &n, &error);
    CHECK(ok, "create leaf should succeed");
    CHECK(n.nodeId == "n1", "id mismatch");
    CHECK(n.label == "counter", "label mismatch");
    CHECK(n.kind == MemoryValueKind::Integer, "kind mismatch");
    CHECK(n.valueText == "42", "value mismatch");
    PASS();
}

void test_create_leaf_rejects_composite_kind() {
    TEST(create_leaf_rejects_composite_kind);
    MemoryValueNode n;
    std::string error;
    const bool ok = MemoryInspectorUiModel::createLeaf("n1", "arr", MemoryValueKind::Container, "[]", &n, &error);
    CHECK(!ok, "create leaf should fail for composite kind");
    CHECK(error == "leaf_kind_must_be_primitive", "wrong error");
    PASS();
}

void test_create_composite_success() {
    TEST(create_composite_success);
    MemoryValueNode n;
    std::string error;
    const bool ok = MemoryInspectorUiModel::createComposite("n2", "record", MemoryValueKind::Struct, &n, &error);
    CHECK(ok, "create composite should succeed");
    CHECK(n.expanded, "composite defaults to expanded");
    CHECK(n.kind == MemoryValueKind::Struct, "kind mismatch");
    PASS();
}

void test_create_composite_rejects_primitive_kind() {
    TEST(create_composite_rejects_primitive_kind);
    MemoryValueNode n;
    std::string error;
    const bool ok = MemoryInspectorUiModel::createComposite("n2", "count", MemoryValueKind::Integer, &n, &error);
    CHECK(!ok, "create composite should fail for primitive kind");
    CHECK(error == "composite_kind_required", "wrong error");
    PASS();
}

void test_append_child_success() {
    TEST(append_child_success);
    MemoryValueNode parent;
    MemoryValueNode child;
    std::string error;
    CHECK(MemoryInspectorUiModel::createComposite("p", "root", MemoryValueKind::Struct, &parent, &error), "parent create failed");
    CHECK(MemoryInspectorUiModel::createLeaf("c", "x", MemoryValueKind::Integer, "3", &child, &error), "child create failed");
    CHECK(MemoryInspectorUiModel::appendChild(&parent, child, &error), "append child should succeed");
    CHECK(parent.children.size() == 1, "expected one child");
    CHECK(parent.children[0].nodeId == "c", "child id mismatch");
    PASS();
}

void test_append_child_rejects_non_composite_parent() {
    TEST(append_child_rejects_non_composite_parent);
    MemoryValueNode parent;
    MemoryValueNode child;
    std::string error;
    CHECK(MemoryInspectorUiModel::createLeaf("p", "value", MemoryValueKind::Integer, "1", &parent, &error), "parent create failed");
    CHECK(MemoryInspectorUiModel::createLeaf("c", "x", MemoryValueKind::Integer, "3", &child, &error), "child create failed");
    CHECK(!MemoryInspectorUiModel::appendChild(&parent, child, &error), "append should fail");
    CHECK(error == "parent_is_not_composite", "wrong error");
    PASS();
}

void test_collect_visible_rows_honors_expanded_state() {
    TEST(collect_visible_rows_honors_expanded_state);
    MemoryValueNode root;
    MemoryValueNode child;
    std::string error;
    CHECK(MemoryInspectorUiModel::createComposite("root", "Root", MemoryValueKind::Struct, &root, &error), "root create failed");
    CHECK(MemoryInspectorUiModel::createLeaf("child", "counter", MemoryValueKind::Integer, "9", &child, &error), "child create failed");
    CHECK(MemoryInspectorUiModel::appendChild(&root, child, &error), "append failed");
    root.expanded = false;

    const auto rows = MemoryInspectorUiModel::collectVisibleRows(root, 10);
    CHECK(rows.size() == 1, "collapsed root should hide children");
    CHECK(rows[0].nodeId == "root", "root row expected");
    PASS();
}

void test_collect_visible_rows_depth_and_limit() {
    TEST(collect_visible_rows_depth_and_limit);
    MemoryValueNode root;
    MemoryValueNode childA;
    MemoryValueNode childB;
    std::string error;
    CHECK(MemoryInspectorUiModel::createComposite("root", "Root", MemoryValueKind::Container, &root, &error), "root create failed");
    CHECK(MemoryInspectorUiModel::createLeaf("a", "a", MemoryValueKind::Integer, "1", &childA, &error), "a create failed");
    CHECK(MemoryInspectorUiModel::createLeaf("b", "b", MemoryValueKind::Integer, "2", &childB, &error), "b create failed");
    CHECK(MemoryInspectorUiModel::appendChild(&root, childA, &error), "append a failed");
    CHECK(MemoryInspectorUiModel::appendChild(&root, childB, &error), "append b failed");

    const auto limitedRows = MemoryInspectorUiModel::collectVisibleRows(root, 2);
    CHECK(limitedRows.size() == 2, "row limit should apply");
    CHECK(limitedRows[1].depth == 1, "child row depth should be one");
    PASS();
}

void test_type_name_mapping_includes_pointer_and_struct() {
    TEST(type_name_mapping_includes_pointer_and_struct);
    CHECK(MemoryInspectorUiModel::typeNameForKind(MemoryValueKind::Pointer) == "pointer", "pointer type mismatch");
    CHECK(MemoryInspectorUiModel::typeNameForKind(MemoryValueKind::Struct) == "struct", "struct type mismatch");
    PASS();
}

void test_resolve_pointer_region_success() {
    TEST(resolve_pointer_region_success);
    MemorySnapshot snapshot;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap", "sess", "thread", 1, &snapshot, &error), "snapshot create failed");
    CHECK(MemorySnapshotModel::addRegion(&snapshot, {"heap-1", MemoryRegionKind::Heap, 0x1000, 64, false}, &error), "region add failed");

    MemoryValueNode pointerNode;
    CHECK(MemoryInspectorUiModel::createLeaf("ptr", "next", MemoryValueKind::Pointer, "0x1010", &pointerNode, &error), "pointer node create failed");

    const MemoryRegionRecord* resolved = nullptr;
    const bool ok = MemoryInspectorUiModel::resolvePointerRegion(snapshot, pointerNode, &resolved, &error);
    CHECK(ok, "pointer region resolve should succeed");
    CHECK(resolved != nullptr && resolved->id == "heap-1", "resolved region mismatch");
    PASS();
}

void test_resolve_pointer_region_rejects_non_pointer_node() {
    TEST(resolve_pointer_region_rejects_non_pointer_node);
    MemorySnapshot snapshot;
    MemoryValueNode valueNode;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap", "sess", "thread", 1, &snapshot, &error), "snapshot create failed");
    CHECK(MemoryInspectorUiModel::createLeaf("v", "x", MemoryValueKind::Integer, "7", &valueNode, &error), "value node create failed");

    const MemoryRegionRecord* resolved = nullptr;
    const bool ok = MemoryInspectorUiModel::resolvePointerRegion(snapshot, valueNode, &resolved, &error);
    CHECK(!ok, "resolve should fail for non-pointer");
    CHECK(error == "node_is_not_pointer", "wrong error");
    PASS();
}

void test_resolve_pointer_region_rejects_bad_pointer_text() {
    TEST(resolve_pointer_region_rejects_bad_pointer_text);
    MemorySnapshot snapshot;
    MemoryValueNode pointerNode;
    std::string error;
    CHECK(MemorySnapshotModel::createSnapshot("snap", "sess", "thread", 1, &snapshot, &error), "snapshot create failed");
    CHECK(MemoryInspectorUiModel::createLeaf("ptr", "next", MemoryValueKind::Pointer, "not-an-address", &pointerNode, &error), "pointer node create failed");

    const MemoryRegionRecord* resolved = nullptr;
    const bool ok = MemoryInspectorUiModel::resolvePointerRegion(snapshot, pointerNode, &resolved, &error);
    CHECK(!ok, "resolve should fail for parse error");
    CHECK(error == "pointer_value_parse_failed", "wrong error");
    PASS();
}

int main() {
    std::cout << "Step 565: Memory Inspector UI Model\n";

    test_create_leaf_success();                             // 1
    test_create_leaf_rejects_composite_kind();             // 2
    test_create_composite_success();                        // 3
    test_create_composite_rejects_primitive_kind();        // 4
    test_append_child_success();                            // 5
    test_append_child_rejects_non_composite_parent();      // 6
    test_collect_visible_rows_honors_expanded_state();     // 7
    test_collect_visible_rows_depth_and_limit();           // 8
    test_type_name_mapping_includes_pointer_and_struct();  // 9
    test_resolve_pointer_region_success();                 // 10
    test_resolve_pointer_region_rejects_non_pointer_node();// 11
    test_resolve_pointer_region_rejects_bad_pointer_text();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
