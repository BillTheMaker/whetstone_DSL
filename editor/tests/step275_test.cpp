// Step 275: Async, Parallelism & Error Handling Annotations (12 tests)
// Tests: ExecAnnotation, BlockingAnnotation, ParallelAnnotation,
//        TrapAnnotation, ExceptionAnnotation, PanicAnnotation
//        + JSON roundtrip + compact AST

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

// --- Test 1: ExecAnnotation creation ---
static void test_exec_annotation() {
    ExecAnnotation ea;
    check(ea.conceptType == "ExecAnnotation", "conceptType is ExecAnnotation");
    ea.mode = "async";
    ea.runtimeHint = "tokio";
    check(ea.mode == "async", "mode field set to async");
    check(ea.runtimeHint == "tokio", "runtimeHint field set to tokio");
}

// --- Test 2: BlockingAnnotation creation ---
static void test_blocking_annotation() {
    BlockingAnnotation ba;
    check(ba.conceptType == "BlockingAnnotation", "conceptType is BlockingAnnotation");
    ba.kind = "io";
    check(ba.kind == "io", "kind field set to io");
}

// --- Test 3: ParallelAnnotation creation ---
static void test_parallel_annotation() {
    ParallelAnnotation pa;
    check(pa.conceptType == "ParallelAnnotation", "conceptType is ParallelAnnotation");
    pa.kind = "data";
    check(pa.kind == "data", "kind field set to data");
}

// --- Test 4: TrapAnnotation, ExceptionAnnotation, PanicAnnotation ---
static void test_error_annotations() {
    TrapAnnotation ta;
    check(ta.conceptType == "TrapAnnotation", "TrapAnnotation conceptType");
    ta.signal = "SIGSEGV";
    check(ta.signal == "SIGSEGV", "signal set to SIGSEGV");

    ExceptionAnnotation ea;
    check(ea.conceptType == "ExceptionAnnotation", "ExceptionAnnotation conceptType");
    ea.style = "checked";
    check(ea.style == "checked", "style set to checked");

    PanicAnnotation pa;
    check(pa.conceptType == "PanicAnnotation", "PanicAnnotation conceptType");
    pa.behavior = "unwind";
    check(pa.behavior == "unwind", "behavior set to unwind");
}

// --- Test 5: ExecAnnotation JSON roundtrip ---
static void test_exec_json_roundtrip() {
    auto* ea = new ExecAnnotation();
    ea->id = "e1";
    ea->mode = "event";
    ea->runtimeHint = "libuv";
    json j = toJson(ea);
    check(j["properties"]["mode"] == "event", "JSON mode correct");
    check(j["properties"]["runtimeHint"] == "libuv", "JSON runtimeHint correct");
    ASTNode* restored = fromJson(j);
    auto* rea = dynamic_cast<ExecAnnotation*>(restored);
    check(rea != nullptr, "fromJson creates ExecAnnotation");
    check(rea->mode == "event" && rea->runtimeHint == "libuv",
          "roundtrip fields preserved");
    delete ea;
    delete restored;
}

// --- Test 6: PanicAnnotation JSON roundtrip ---
static void test_panic_json_roundtrip() {
    auto* pa = new PanicAnnotation();
    pa->id = "p1";
    pa->behavior = "abort";
    json j = toJson(pa);
    ASTNode* restored = fromJson(j);
    auto* rpa = dynamic_cast<PanicAnnotation*>(restored);
    check(rpa != nullptr, "fromJson creates PanicAnnotation");
    check(rpa->behavior == "abort", "roundtrip behavior preserved");
    delete pa;
    delete restored;
}

// --- Test 7: createNode factory for Step 275 types ---
static void test_create_node_factory() {
    std::vector<std::string> types = {
        "ExecAnnotation", "BlockingAnnotation", "ParallelAnnotation",
        "TrapAnnotation", "ExceptionAnnotation", "PanicAnnotation"
    };
    for (const auto& t : types) {
        ASTNode* node = createNode(t);
        check(node != nullptr && node->conceptType == t,
              "createNode(" + t + ") works");
        delete node;
    }
}

// --- Test 8: Compact AST shows exec in semantic summary ---
static void test_compact_ast_exec() {
    auto* func = new Function();
    func->id = "f1";
    func->name = "fetchData";
    auto* ea = new ExecAnnotation();
    ea->mode = "async";
    ea->runtimeHint = "tokio";
    func->addChild("annotations", ea);
    json sem = extractSemanticSummary(func);
    check(sem.contains("exec"), "exec in semantic summary");
    check(sem["exec"]["mode"] == "async", "exec mode correct");
    check(sem["exec"]["runtime"] == "tokio", "exec runtime correct");
    delete func;
}

// --- Test 9: Compact AST shows parallel and blocking ---
static void test_compact_ast_parallel_blocking() {
    auto* func = new Function();
    func->id = "f2";
    func->name = "processArray";
    auto* pa = new ParallelAnnotation(); pa->kind = "data";
    auto* ba = new BlockingAnnotation(); ba->kind = "compute";
    func->addChild("annotations", pa);
    func->addChild("annotations", ba);
    json sem = extractSemanticSummary(func);
    check(sem["parallel"] == "data", "parallel in semantic summary");
    check(sem["blocking"] == "compute", "blocking in semantic summary");
    delete func;
}

// --- Test 10: Compact AST shows error handling annotations ---
static void test_compact_ast_error_handling() {
    auto* func = new Function();
    func->id = "f3";
    func->name = "riskyOp";
    auto* ta = new TrapAnnotation(); ta->signal = "SIGFPE";
    auto* ea = new ExceptionAnnotation(); ea->style = "unchecked";
    auto* pa = new PanicAnnotation(); pa->behavior = "unwind";
    func->addChild("annotations", ta);
    func->addChild("annotations", ea);
    func->addChild("annotations", pa);
    json sem = extractSemanticSummary(func);
    check(sem["trap"] == "SIGFPE", "trap in semantic summary");
    check(sem["exception"] == "unchecked", "exception in semantic summary");
    check(sem["panic"] == "unwind", "panic in semantic summary");
    delete func;
}

// --- Test 11: All 6 async/error annotations on one function ---
static void test_all_async_error_annotations() {
    auto* func = new Function();
    func->id = "f4";
    func->name = "fullAsync";
    func->addChild("annotations", [&]{ auto* a = new ExecAnnotation(); a->mode = "async"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new BlockingAnnotation(); a->kind = "io"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new ParallelAnnotation(); a->kind = "task"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new TrapAnnotation(); a->signal = "SIGSEGV"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new ExceptionAnnotation(); a->style = "checked"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new PanicAnnotation(); a->behavior = "abort"; return a; }());
    json sem = extractSemanticSummary(func);
    check(sem.contains("exec") && sem.contains("blocking") &&
          sem.contains("parallel") && sem.contains("trap") &&
          sem.contains("exception") && sem.contains("panic"),
          "all 6 async/error annotations in semantic summary");
    delete func;
}

// --- Test 12: TrapAnnotation and ExceptionAnnotation JSON roundtrip ---
static void test_trap_exception_roundtrip() {
    auto* ta = new TrapAnnotation();
    ta->id = "t1";
    ta->signal = "SIGABRT";
    json jt = toJson(ta);
    ASTNode* rt = fromJson(jt);
    auto* rta = dynamic_cast<TrapAnnotation*>(rt);
    check(rta != nullptr && rta->signal == "SIGABRT", "TrapAnnotation roundtrip");

    auto* ea = new ExceptionAnnotation();
    ea->id = "ex1";
    ea->style = "checked";
    json je = toJson(ea);
    ASTNode* re = fromJson(je);
    auto* rea = dynamic_cast<ExceptionAnnotation*>(re);
    check(rea != nullptr && rea->style == "checked", "ExceptionAnnotation roundtrip");

    delete ta; delete rt; delete ea; delete re;
}

int main() {
    std::cout << "=== Step 275: Async, Parallelism & Error Handling Annotations ===\n";
    test_exec_annotation();
    test_blocking_annotation();
    test_parallel_annotation();
    test_error_annotations();
    test_exec_json_roundtrip();
    test_panic_json_roundtrip();
    test_create_node_factory();
    test_compact_ast_exec();
    test_compact_ast_parallel_blocking();
    test_compact_ast_error_handling();
    test_all_async_error_annotations();
    test_trap_exception_roundtrip();
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed out of "
              << (passed + failed) << "\n";
    return failed > 0 ? 1 : 0;
}
