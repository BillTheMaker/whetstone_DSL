// Step 277: Phase 10c Integration Tests (8 tests)
// Cross-subject annotation workflows: type + concurrency + scope on AST,
// compact AST, sidecar roundtrip, RPC set/get for new types.

#include <cassert>
#include <iostream>
#include <string>
#include <filesystem>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include "ASTUtils.h"
#include "CompactAST.h"
#include "SidecarPersistence.h"
#include "HeadlessEditorState.h"
#include <memory>

static int passed = 0;
static int failed = 0;

static void check(bool cond, const std::string& name) {
    if (cond) { std::cout << "  PASS: " << name << "\n"; ++passed; }
    else      { std::cout << "  FAIL: " << name << "\n"; ++failed; }
}

// Helper: build a simple function AST
static Module* buildTestAST() {
    auto* mod = new Module();
    mod->id = "mod1";
    mod->name = "test_module";

    auto* fn = new Function();
    fn->id = "fn1";
    fn->name = "processData";
    mod->addChild("functions", fn);

    auto* fn2 = new Function();
    fn2->id = "fn2";
    fn2->name = "transformItem";
    mod->addChild("functions", fn2);

    return mod;
}

// Helper: invoke RPC
static json rpc(HeadlessEditorState& state, const std::string& method,
                const json& params = {}, const std::string& session = "s1") {
    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}};
    if (!params.empty()) req["params"] = params;
    return handleHeadlessAgentRequest(state, req, session);
}

// --- Test 1: Type annotations drive compact AST output ---
static void test_type_annotations_compact_ast() {
    auto* mod = buildTestAST();
    auto* fn = findNodeById(mod, "fn1");

    // Add type annotations
    auto* bw = new BitWidthAnnotation(); bw->width = 32;
    auto* la = new LayoutAnnotation(); la->mode = "aligned"; la->alignment = 8;
    auto* mu = new MutAnnotation(); mu->depth = "deep";
    fn->addChild("annotations", bw);
    fn->addChild("annotations", la);
    fn->addChild("annotations", mu);

    json compact = toJsonCompactSummary(mod);
    // Find fn1 in compact summary
    bool found = false;
    for (const auto& node : compact) {
        if (node.value("id", "") == "fn1" && node.contains("semantic")) {
            auto& sem = node["semantic"];
            found = sem.contains("bitWidth") && sem.contains("layout") &&
                    sem.contains("mutability");
        }
    }
    check(found, "type annotations appear in compact AST summary");
    delete mod;
}

// --- Test 2: Concurrency annotations in compact AST ---
static void test_concurrency_annotations_compact_ast() {
    auto* mod = buildTestAST();
    auto* fn = findNodeById(mod, "fn1");

    auto* aa = new AtomicAnnotation(); aa->consistency = "seq_cst";
    auto* sa = new SyncAnnotation(); sa->primitive = "monitor";
    fn->addChild("annotations", aa);
    fn->addChild("annotations", sa);

    json compact = toJsonCompactSummary(mod);
    bool found = false;
    for (const auto& node : compact) {
        if (node.value("id", "") == "fn1" && node.contains("semantic")) {
            auto& sem = node["semantic"];
            found = sem.contains("atomic") && sem.contains("sync");
        }
    }
    check(found, "concurrency annotations appear in compact AST summary");
    delete mod;
}

// --- Test 3: Scope annotations in compact AST ---
static void test_scope_annotations_compact_ast() {
    auto* mod = buildTestAST();
    auto* fn = findNodeById(mod, "fn2");

    auto* ca = new CaptureAnnotation(); ca->strategy = "ref";
    auto* va = new VisibilityAnnotation(); va->level = "private";
    fn->addChild("annotations", ca);
    fn->addChild("annotations", va);

    json compact = toJsonCompactSummary(mod);
    bool found = false;
    for (const auto& node : compact) {
        if (node.value("id", "") == "fn2" && node.contains("semantic")) {
            auto& sem = node["semantic"];
            found = sem.contains("capture") && sem.contains("visibility");
        }
    }
    check(found, "scope annotations appear in compact AST summary");
    delete mod;
}

// --- Test 4: All Subject 2-4 annotations survive sidecar save/load ---
static void test_sidecar_roundtrip() {
    namespace fs = std::filesystem;
    std::string workspace = "/tmp/step277_test_ws";
    fs::create_directories(workspace);

    auto* mod = buildTestAST();
    auto* fn1 = findNodeById(mod, "fn1");
    auto* fn2 = findNodeById(mod, "fn2");

    // Add one annotation from each subject
    auto* bw = new BitWidthAnnotation(); bw->width = 64;
    fn1->addChild("annotations", bw);
    auto* aa = new AtomicAnnotation(); aa->consistency = "relaxed";
    fn1->addChild("annotations", aa);
    auto* ba = new BindingAnnotation(); ba->time = "static";
    fn2->addChild("annotations", ba);

    auto saveResult = saveSidecarAST(workspace, "test.wh", mod);
    check(saveResult.success, "sidecar save succeeds");
    check(saveResult.annotationCount == 3, "3 annotations saved");

    // Build fresh AST and load
    auto* mod2 = buildTestAST();
    auto loadResult = loadSidecarAST(workspace, "test.wh", mod2);
    check(loadResult.success, "sidecar load succeeds");
    check(loadResult.mergedCount == 3, "3 annotations merged");

    // Verify annotations on reloaded AST
    auto* rfn1 = findNodeById(mod2, "fn1");
    auto annos1 = rfn1->getChildren("annotations");
    bool hasBitWidth = false, hasAtomic = false;
    for (auto* a : annos1) {
        if (a->conceptType == "BitWidthAnnotation") hasBitWidth = true;
        if (a->conceptType == "AtomicAnnotation") hasAtomic = true;
    }
    check(hasBitWidth && hasAtomic, "type and concurrency annotations survive roundtrip");

    delete mod;
    delete mod2;
    fs::remove_all(workspace);
}

// --- Test 5: Mixed annotations on single function ---
static void test_mixed_annotations_single_function() {
    auto* func = new Function();
    func->id = "f1";
    func->name = "mixedOp";

    // Subject 2: type
    auto* bw = new BitWidthAnnotation(); bw->width = 16;
    auto* id = new IdentityAnnotation(); id->mode = "structural";
    // Subject 3: concurrency
    auto* tm = new ThreadModelAnnotation(); tm->model = "green";
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    // Subject 4: scope
    auto* va = new VisibilityAnnotation(); va->level = "public";
    auto* ca = new CaptureAnnotation(); ca->strategy = "value";

    func->addChild("annotations", bw);
    func->addChild("annotations", id);
    func->addChild("annotations", tm);
    func->addChild("annotations", ea);
    func->addChild("annotations", va);
    func->addChild("annotations", ca);

    json sem = extractSemanticSummary(func);
    check(sem.contains("bitWidth") && sem.contains("identity") &&
          sem.contains("threadModel") && sem.contains("exec") &&
          sem.contains("visibility") && sem.contains("capture"),
          "mixed annotations from 3 subjects on single function");
    delete func;
}

// --- Test 6: getSemanticAnnotations returns Subject 2-4 types via RPC ---
static void test_rpc_get_annotations() {
    HeadlessEditorState state;
    state.openBuffer("test.wh", "", "python");
    auto* mod = buildTestAST();
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));
    state.setAgentRole("s1", AgentRole::Refactor);

    // Set type annotation via RPC
    auto r1 = rpc(state, "setSemanticAnnotation", {
        {"nodeId", "fn1"}, {"type", "bitWidth"}, {"fields", {{"width", 32}}}
    });
    check(r1.contains("result"), "setSemanticAnnotation bitWidth via RPC");

    // Set concurrency annotation via RPC
    auto r2 = rpc(state, "setSemanticAnnotation", {
        {"nodeId", "fn1"}, {"type", "atomic"}, {"fields", {{"consistency", "seq_cst"}}}
    });
    check(r2.contains("result"), "setSemanticAnnotation atomic via RPC");

    // Set scope annotation via RPC
    auto r3 = rpc(state, "setSemanticAnnotation", {
        {"nodeId", "fn1"}, {"type", "visibility"}, {"fields", {{"level", "public"}}}
    });
    check(r3.contains("result"), "setSemanticAnnotation visibility via RPC");

    // Query all annotations
    auto r4 = rpc(state, "getSemanticAnnotations", {{"nodeId", "fn1"}});
    auto annos = r4["result"]["annotations"];
    bool hasBW = false, hasAtom = false, hasVis = false;
    for (const auto& a : annos) {
        std::string t = a.value("type", "");
        if (t == "BitWidthAnnotation") hasBW = true;
        if (t == "AtomicAnnotation") hasAtom = true;
        if (t == "VisibilityAnnotation") hasVis = true;
    }
    check(hasBW && hasAtom && hasVis,
          "getSemanticAnnotations returns Subject 2-4 types");
}

// --- Test 7: setSemanticAnnotation works for all new type keys ---
static void test_rpc_set_all_new_types() {
    HeadlessEditorState state;
    state.openBuffer("test.wh", "", "python");
    auto* mod = buildTestAST();
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));
    state.setAgentRole("s1", AgentRole::Refactor);

    // Test a representative selection of new type keys
    std::vector<std::pair<std::string, json>> tests = {
        {"endian", {{"order", "big"}}},
        {"layout", {{"mode", "packed"}, {"alignment", 4}}},
        {"nullability", {{"nullable", true}, {"strategy", "strict"}}},
        {"variance", {{"variance", "covariant"}}},
        {"identity", {{"mode", "nominal"}}},
        {"mut", {{"depth", "deep"}}},
        {"typeState", {{"state", "reified"}}},
        {"sync", {{"primitive", "monitor"}}},
        {"threadModel", {{"model", "os"}}},
        {"memoryBarrier", json::object()},
        {"exec", {{"mode", "async"}, {"runtimeHint", "tokio"}}},
        {"blocking", {{"kind", "io"}}},
        {"parallel", {{"kind", "data"}}},
        {"trap", {{"signal", "SIGSEGV"}}},
        {"exception", {{"style", "checked"}}},
        {"panic", {{"behavior", "abort"}}},
        {"binding", {{"time", "static"}}},
        {"lookup", {{"mode", "lexical"}}},
        {"capture", {{"strategy", "move"}}},
        {"namespace", {{"style", "qualified"}}},
        {"scope", {{"kind", "local"}}},
    };

    int successCount = 0;
    for (const auto& [type, fields] : tests) {
        auto r = rpc(state, "setSemanticAnnotation", {
            {"nodeId", "fn2"}, {"type", type}, {"fields", fields}
        });
        if (r.contains("result") && r["result"].value("success", false))
            ++successCount;
    }
    check(successCount == (int)tests.size(),
          "all 21 new type keys accepted by setSemanticAnnotation RPC");
}

// --- Test 8: isSemanticAnnotation recognizes all new types ---
static void test_is_semantic_annotation() {
    std::vector<std::string> allTypes = {
        "BitWidthAnnotation", "EndianAnnotation", "LayoutAnnotation",
        "NullabilityAnnotation", "VarianceAnnotation",
        "IdentityAnnotation", "MutAnnotation", "TypeStateAnnotation",
        "AtomicAnnotation", "SyncAnnotation", "ThreadModelAnnotation",
        "MemoryBarrierAnnotation",
        "ExecAnnotation", "BlockingAnnotation", "ParallelAnnotation",
        "TrapAnnotation", "ExceptionAnnotation", "PanicAnnotation",
        "BindingAnnotation", "LookupAnnotation", "CaptureAnnotation",
        "VisibilityAnnotation", "NamespaceAnnotation", "ScopeAnnotation"
    };
    int recognized = 0;
    for (const auto& t : allTypes) {
        if (isSemanticAnnotation(t)) ++recognized;
    }
    check(recognized == 24, "isSemanticAnnotation recognizes all 24 new types");
    check(!isSemanticAnnotation("DerefStrategy"),
          "isSemanticAnnotation rejects non-semantic DerefStrategy");
}

int main() {
    std::cout << "=== Step 277: Phase 10c Integration Tests ===\n";
    test_type_annotations_compact_ast();
    test_concurrency_annotations_compact_ast();
    test_scope_annotations_compact_ast();
    test_sidecar_roundtrip();
    test_mixed_annotations_single_function();
    test_rpc_get_annotations();
    test_rpc_set_all_new_types();
    test_is_semantic_annotation();
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed out of "
              << (passed + failed) << "\n";
    return failed > 0 ? 1 : 0;
}
