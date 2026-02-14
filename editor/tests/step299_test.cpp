// Step 299: Cross-Type Conflict Detection Tests (12 tests)
// Tests collectCrossTypeConflicts() from AnnotationConflictExtended.h.

#include "AnnotationConflictExtended.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasConflict(const std::vector<CrossTypeConflict>& conflicts,
                        const std::string& t1, const std::string& t2) {
    for (const auto& c : conflicts)
        if ((c.type1 == t1 && c.type2 == t2) || (c.type1 == t2 && c.type2 == t1))
            return true;
    return false;
}

// 1. Pure + Blocking detected
void test_pure_blocking() {
    TEST(pure_blocking_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    fn->addChild("annotations", new PureAnnotation());
    auto bl = new BlockingAnnotation(); bl->kind = "io";
    fn->addChild("annotations", bl);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(hasConflict(conflicts, "PureAnnotation", "BlockingAnnotation"),
          "expected Pure+Blocking conflict");
    PASS();
}

// 2. Atomic + Sync detected
void test_atomic_sync() {
    TEST(atomic_sync_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto at = new AtomicAnnotation(); at->consistency = "seq_cst";
    fn->addChild("annotations", at);
    auto sy = new SyncAnnotation(); sy->primitive = "monitor";
    fn->addChild("annotations", sy);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(hasConflict(conflicts, "AtomicAnnotation", "SyncAnnotation"),
          "expected Atomic+Sync conflict");
    PASS();
}

// 3. Capture(move) + Owner(Shared_ARC) detected
void test_capture_move_owner_shared() {
    TEST(capture_move_owner_shared_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto cap = new CaptureAnnotation(); cap->strategy = "move";
    fn->addChild("annotations", cap);
    auto own = new OwnerAnnotation(); own->strategy = "Shared_ARC";
    fn->addChild("annotations", own);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(hasConflict(conflicts, "CaptureAnnotation", "OwnerAnnotation"),
          "expected Capture(move)+Owner(Shared_ARC) conflict");
    PASS();
}

// 4. Capture(value) + Owner(Shared_ARC) -> no conflict
void test_capture_value_owner_shared() {
    TEST(capture_value_owner_shared_no_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto cap = new CaptureAnnotation(); cap->strategy = "value";
    fn->addChild("annotations", cap);
    auto own = new OwnerAnnotation(); own->strategy = "Shared_ARC";
    fn->addChild("annotations", own);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(!hasConflict(conflicts, "CaptureAnnotation", "OwnerAnnotation"),
          "should not trigger conflict for value+Shared_ARC");
    PASS();
}

// 5. ConstExpr + Exec(async) detected
void test_constexpr_exec_async() {
    TEST(constexpr_exec_async_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    fn->addChild("annotations", new ConstExprAnnotation());
    auto exec = new ExecAnnotation(); exec->mode = "async";
    fn->addChild("annotations", exec);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(hasConflict(conflicts, "ConstExprAnnotation", "ExecAnnotation"),
          "expected ConstExpr+Exec(async) conflict");
    PASS();
}

// 6. ConstExpr + Exec(event) -> no conflict
void test_constexpr_exec_event() {
    TEST(constexpr_exec_event_no_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    fn->addChild("annotations", new ConstExprAnnotation());
    auto exec = new ExecAnnotation(); exec->mode = "event";
    fn->addChild("annotations", exec);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(!hasConflict(conflicts, "ConstExprAnnotation", "ExecAnnotation"),
          "should not trigger conflict for ConstExpr+event");
    PASS();
}

// 7. Inline(Always) + TailCall detected
void test_inline_always_tailcall() {
    TEST(inline_always_tailcall_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto inl = new InlineAnnotation(); inl->mode = "Always";
    fn->addChild("annotations", inl);
    fn->addChild("annotations", new TailCallAnnotation());
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(hasConflict(conflicts, "InlineAnnotation", "TailCallAnnotation"),
          "expected Inline(Always)+TailCall conflict");
    PASS();
}

// 8. Pack + Layout(aligned) detected
void test_pack_layout_aligned() {
    TEST(pack_layout_aligned_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    fn->addChild("annotations", new PackAnnotation());
    auto lay = new LayoutAnnotation(); lay->mode = "aligned";
    fn->addChild("annotations", lay);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(hasConflict(conflicts, "PackAnnotation", "LayoutAnnotation"),
          "expected Pack+Layout(aligned) conflict");
    PASS();
}

// 9. Pack + Layout(packed) -> no conflict
void test_pack_layout_packed() {
    TEST(pack_layout_packed_no_conflict);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    fn->addChild("annotations", new PackAnnotation());
    auto lay = new LayoutAnnotation(); lay->mode = "packed";
    fn->addChild("annotations", lay);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(!hasConflict(conflicts, "PackAnnotation", "LayoutAnnotation"),
          "should not trigger conflict for Pack+Layout(packed)");
    PASS();
}

// 10. No conflicts on clean node -> empty
void test_no_conflicts_clean() {
    TEST(no_conflicts_clean_node);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto bw = new BitWidthAnnotation(); bw->width = 32;
    fn->addChild("annotations", bw);
    auto lo = new LoopAnnotation(); lo->hint = "unroll";
    fn->addChild("annotations", lo);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(conflicts.empty(), "expected no conflicts for non-conflicting annotations");
    PASS();
}

// 11. Recursive detection in children
void test_recursive_detection() {
    TEST(recursive_conflict_in_children);
    auto mod = std::make_unique<Module>();
    // Conflict is on a child function, not the module
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    fn->addChild("annotations", new PureAnnotation());
    auto bl = new BlockingAnnotation(); bl->kind = "io";
    fn->addChild("annotations", bl);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(conflicts.size() == 1, "expected 1 conflict from child, got " + std::to_string(conflicts.size()));
    CHECK(conflicts[0].nodeId == "fn1", "conflict should be on fn1");
    PASS();
}

// 12. Multiple conflicts on same node
void test_multiple_conflicts() {
    TEST(multiple_conflicts_same_node);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    // Pure + Blocking
    fn->addChild("annotations", new PureAnnotation());
    auto bl = new BlockingAnnotation(); bl->kind = "io";
    fn->addChild("annotations", bl);
    // Atomic + Sync
    auto at = new AtomicAnnotation(); at->consistency = "seq_cst";
    fn->addChild("annotations", at);
    auto sy = new SyncAnnotation(); sy->primitive = "monitor";
    fn->addChild("annotations", sy);
    mod->addChild("functions", fn);

    std::vector<CrossTypeConflict> conflicts;
    collectCrossTypeConflicts(mod.get(), conflicts);
    CHECK(conflicts.size() >= 2, "expected at least 2 conflicts, got " + std::to_string(conflicts.size()));
    CHECK(hasConflict(conflicts, "PureAnnotation", "BlockingAnnotation"), "missing Pure+Blocking");
    CHECK(hasConflict(conflicts, "AtomicAnnotation", "SyncAnnotation"), "missing Atomic+Sync");
    PASS();
}

int main() {
    std::cout << "=== Step 299: Cross-Type Conflict Detection Tests ===\n";
    test_pure_blocking();
    test_atomic_sync();
    test_capture_move_owner_shared();
    test_capture_value_owner_shared();
    test_constexpr_exec_async();
    test_constexpr_exec_event();
    test_inline_always_tailcall();
    test_pack_layout_aligned();
    test_pack_layout_packed();
    test_no_conflicts_clean();
    test_recursive_detection();
    test_multiple_conflicts();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
