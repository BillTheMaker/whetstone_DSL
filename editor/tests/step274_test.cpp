// Step 274: Concurrency Annotations — Primitives & Memory Model (12 tests)
// Tests: AtomicAnnotation, SyncAnnotation, ThreadModelAnnotation,
//        MemoryBarrierAnnotation + JSON roundtrip + compact AST

#include <cassert>
#include <iostream>
#include <string>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include "ASTUtils.h"
#include "CompactAST.h"

static int passed = 0;
static int failed = 0;

static void check(bool cond, const std::string& name) {
    if (cond) { std::cout << "  PASS: " << name << "\n"; ++passed; }
    else      { std::cout << "  FAIL: " << name << "\n"; ++failed; }
}

// --- Test 1: AtomicAnnotation creation ---
static void test_atomic_annotation() {
    AtomicAnnotation aa;
    check(aa.conceptType == "AtomicAnnotation", "conceptType is AtomicAnnotation");
    aa.consistency = "seq_cst";
    check(aa.consistency == "seq_cst", "consistency field set to seq_cst");
}

// --- Test 2: SyncAnnotation creation ---
static void test_sync_annotation() {
    SyncAnnotation sa;
    check(sa.conceptType == "SyncAnnotation", "conceptType is SyncAnnotation");
    sa.primitive = "monitor";
    check(sa.primitive == "monitor", "primitive field set to monitor");
}

// --- Test 3: ThreadModelAnnotation creation ---
static void test_threadmodel_annotation() {
    ThreadModelAnnotation tm;
    check(tm.conceptType == "ThreadModelAnnotation", "conceptType is ThreadModelAnnotation");
    tm.model = "green";
    check(tm.model == "green", "model field set to green");
}

// --- Test 4: MemoryBarrierAnnotation creation (marker, no fields) ---
static void test_memorybarrier_annotation() {
    MemoryBarrierAnnotation mb;
    check(mb.conceptType == "MemoryBarrierAnnotation", "conceptType is MemoryBarrierAnnotation");
}

// --- Test 5: AtomicAnnotation JSON roundtrip ---
static void test_atomic_json_roundtrip() {
    auto* aa = new AtomicAnnotation();
    aa->id = "a1";
    aa->consistency = "relaxed";
    json j = toJson(aa);
    check(j["properties"]["consistency"] == "relaxed", "JSON consistency correct");
    ASTNode* restored = fromJson(j);
    auto* raa = dynamic_cast<AtomicAnnotation*>(restored);
    check(raa != nullptr, "fromJson creates AtomicAnnotation");
    check(raa->consistency == "relaxed", "roundtrip consistency preserved");
    delete aa;
    delete restored;
}

// --- Test 6: SyncAnnotation JSON roundtrip ---
static void test_sync_json_roundtrip() {
    auto* sa = new SyncAnnotation();
    sa->id = "s1";
    sa->primitive = "semaphore";
    json j = toJson(sa);
    ASTNode* restored = fromJson(j);
    auto* rsa = dynamic_cast<SyncAnnotation*>(restored);
    check(rsa != nullptr, "fromJson creates SyncAnnotation");
    check(rsa->primitive == "semaphore", "roundtrip primitive preserved");
    delete sa;
    delete restored;
}

// --- Test 7: ThreadModelAnnotation JSON roundtrip ---
static void test_threadmodel_json_roundtrip() {
    auto* tm = new ThreadModelAnnotation();
    tm->id = "tm1";
    tm->model = "fiber";
    json j = toJson(tm);
    ASTNode* restored = fromJson(j);
    auto* rtm = dynamic_cast<ThreadModelAnnotation*>(restored);
    check(rtm != nullptr, "fromJson creates ThreadModelAnnotation");
    check(rtm->model == "fiber", "roundtrip model preserved");
    delete tm;
    delete restored;
}

// --- Test 8: MemoryBarrierAnnotation JSON roundtrip ---
static void test_memorybarrier_json_roundtrip() {
    auto* mb = new MemoryBarrierAnnotation();
    mb->id = "mb1";
    json j = toJson(mb);
    check(j["concept"] == "MemoryBarrierAnnotation", "JSON concept correct");
    ASTNode* restored = fromJson(j);
    auto* rmb = dynamic_cast<MemoryBarrierAnnotation*>(restored);
    check(rmb != nullptr, "fromJson creates MemoryBarrierAnnotation");
    delete mb;
    delete restored;
}

// --- Test 9: createNode factory for Step 274 types ---
static void test_create_node_factory() {
    std::vector<std::string> types = {
        "AtomicAnnotation", "SyncAnnotation",
        "ThreadModelAnnotation", "MemoryBarrierAnnotation"
    };
    for (const auto& t : types) {
        ASTNode* node = createNode(t);
        check(node != nullptr && node->conceptType == t,
              "createNode(" + t + ") works");
        delete node;
    }
}

// --- Test 10: Compact AST shows atomic in semantic summary ---
static void test_compact_ast_atomic() {
    auto* func = new Function();
    func->id = "f1";
    func->name = "incrementCounter";
    auto* aa = new AtomicAnnotation();
    aa->consistency = "seq_cst";
    func->addChild("annotations", aa);
    json sem = extractSemanticSummary(func);
    check(sem.contains("atomic"), "atomic in semantic summary");
    check(sem["atomic"] == "seq_cst", "atomic value correct");
    delete func;
}

// --- Test 11: Compact AST shows memoryBarrier marker ---
static void test_compact_ast_memorybarrier() {
    auto* func = new Function();
    func->id = "f2";
    func->name = "fence";
    auto* mb = new MemoryBarrierAnnotation();
    func->addChild("annotations", mb);
    json sem = extractSemanticSummary(func);
    check(sem.contains("memoryBarrier"), "memoryBarrier in semantic summary");
    check(sem["memoryBarrier"] == true, "memoryBarrier is true");
    delete func;
}

// --- Test 12: All 4 concurrency annotations on one function ---
static void test_all_concurrency_annotations() {
    auto* func = new Function();
    func->id = "f3";
    func->name = "concurrentOp";
    auto* aa = new AtomicAnnotation(); aa->consistency = "acquire";
    auto* sa = new SyncAnnotation(); sa->primitive = "spin";
    auto* tm = new ThreadModelAnnotation(); tm->model = "os";
    auto* mb = new MemoryBarrierAnnotation();
    func->addChild("annotations", aa);
    func->addChild("annotations", sa);
    func->addChild("annotations", tm);
    func->addChild("annotations", mb);
    json sem = extractSemanticSummary(func);
    check(sem.contains("atomic") && sem.contains("sync") &&
          sem.contains("threadModel") && sem.contains("memoryBarrier"),
          "all 4 concurrency annotations in semantic summary");
    delete func;
}

int main() {
    std::cout << "=== Step 274: Concurrency Annotations — Primitives & Memory Model ===\n";
    test_atomic_annotation();
    test_sync_annotation();
    test_threadmodel_annotation();
    test_memorybarrier_annotation();
    test_atomic_json_roundtrip();
    test_sync_json_roundtrip();
    test_threadmodel_json_roundtrip();
    test_memorybarrier_json_roundtrip();
    test_create_node_factory();
    test_compact_ast_atomic();
    test_compact_ast_memorybarrier();
    test_all_concurrency_annotations();
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed out of "
              << (passed + failed) << "\n";
    return failed > 0 ? 1 : 0;
}
