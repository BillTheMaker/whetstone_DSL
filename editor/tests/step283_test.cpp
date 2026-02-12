// Step 283: Phase 10d Integration Tests (8 tests)
#include <cassert>
#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include "ASTUtils.h"
#include "CompactAST.h"
#include "SidecarPersistence.h"
#include "HeadlessEditorState.h"

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

static Module* buildTestAST() {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "test";
    auto* fn1 = new Function(); fn1->id = "fn1"; fn1->name = "ffi_bridge";
    auto* fn2 = new Function(); fn2->id = "fn2"; fn2->name = "optimize";
    mod->addChild("functions", fn1);
    mod->addChild("functions", fn2);
    return mod;
}

static json rpc(HeadlessEditorState& st, const std::string& method,
                const json& params = {}, const std::string& sess = "s1") {
    json req = {{"jsonrpc","2.0"},{"id",1},{"method",method}};
    if (!params.empty()) req["params"] = params;
    return handleHeadlessAgentRequest(st, req, sess);
}

// Test 1: Shim annotations on FFI boundary function
static void test_shim_on_ffi() {
    auto* mod = buildTestAST();
    auto* fn = findNodeById(mod, "fn1");
    auto* ia = new IntrinsicAnnotation(); ia->instruction = "_mm_add"; ia->arch = "sse";
    auto* cc = new CallingConvAnnotation(); cc->convention = "cdecl";
    auto* la = new LinkAnnotation(); la->symbolName = "ext_func"; la->library = "libext.so";
    fn->addChild("annotations", ia);
    fn->addChild("annotations", cc);
    fn->addChild("annotations", la);
    json sem = extractSemanticSummary(fn);
    check(sem.contains("intrinsic") && sem.contains("callingConv") && sem.contains("link"),
          "shim annotations on FFI function");
    delete mod;
}

// Test 2: OriginalAnnotation preserves legacy source
static void test_original_preserves_source() {
    auto* mod = buildTestAST();
    auto* fn = findNodeById(mod, "fn1");
    auto* oa = new OriginalAnnotation();
    oa->sourceCode = "def bridge(x): return ext_call(x)";
    oa->sourceLanguage = "python";
    fn->addChild("annotations", oa);
    json sem = extractSemanticSummary(fn);
    check(sem.contains("original") && sem["original"]["lang"] == "python" &&
          sem["original"]["hasSource"] == true, "original annotation preserves source info");
    delete mod;
}

// Test 3: Optimization annotations in compact AST
static void test_optimization_compact_ast() {
    auto* mod = buildTestAST();
    auto* fn = findNodeById(mod, "fn2");
    auto* la = new LoopAnnotation(); la->hint = "vectorize"; la->factor = 4;
    auto* aa = new AlignAnnotation(); aa->bytes = 32;
    fn->addChild("annotations", la);
    fn->addChild("annotations", aa);
    json compact = toJsonCompactSummary(mod);
    bool found = false;
    for (const auto& n : compact) {
        if (n.value("id","") == "fn2" && n.contains("semantic")) {
            auto& s = n["semantic"];
            found = s.contains("loop") && s.contains("align");
        }
    }
    check(found, "optimization annotations in compact AST summary");
    delete mod;
}

// Test 4: Meta-programming annotations mark macro-expanded code
static void test_meta_marks_generated() {
    auto* f = new Function(); f->id = "f1"; f->name = "generated_fn";
    auto* syn = new SyntheticAnnotation(); syn->generator = "derive_macro"; syn->isStructuralRisk = true;
    auto* meta = new MetaAnnotation(); meta->state = "unquoted"; meta->phase = "compile";
    f->addChild("annotations", syn);
    f->addChild("annotations", meta);
    json sem = extractSemanticSummary(f);
    check(sem.contains("synthetic") && sem["synthetic"]["risk"] == true &&
          sem.contains("meta") && sem["meta"]["phase"] == "compile",
          "meta annotations mark generated code");
    delete f;
}

// Test 5: Policy + Choice + Decision workflow
static void test_policy_choice_decision() {
    auto* f = new Function(); f->id = "f1"; f->name = "sync_op";
    auto* policy = new PolicyAnnotation(); policy->strictness = "high"; policy->perf = "critical";
    auto* choice = new ChoiceAnnotation(); choice->choiceId = "sync1"; choice->options = {"mutex","rwlock","atomic"};
    auto* decision = new DecisionAnnotation(); decision->choiceId = "sync1";
    decision->selection = "atomic"; decision->author = "agent"; decision->reason = "single-writer";
    f->addChild("annotations", policy);
    f->addChild("annotations", choice);
    f->addChild("annotations", decision);
    json sem = extractSemanticSummary(f);
    check(sem.contains("policy") && sem.contains("choice") && sem.contains("decision") &&
          sem["decision"]["selection"] == "atomic",
          "policy → choice → decision workflow");
    delete f;
}

// Test 6: All Subject 5-8 annotations survive sidecar roundtrip
static void test_sidecar_roundtrip() {
    namespace fs = std::filesystem;
    std::string ws = "/tmp/step283_test_ws";
    fs::create_directories(ws);

    auto* mod = buildTestAST();
    auto* fn1 = findNodeById(mod, "fn1");
    auto* fn2 = findNodeById(mod, "fn2");
    // Subject 5
    fn1->addChild("annotations", [&]{ auto* a = new IntrinsicAnnotation(); a->instruction="x"; return a; }());
    fn1->addChild("annotations", [&]{ auto* a = new TargetAnnotation(); a->platform="linux"; return a; }());
    // Subject 6
    fn2->addChild("annotations", [&]{ auto* a = new LoopAnnotation(); a->hint="unroll"; return a; }());
    // Subject 7
    fn2->addChild("annotations", [&]{ auto* a = new MetaAnnotation(); a->state="quoted"; return a; }());
    // Subject 8
    fn1->addChild("annotations", [&]{ auto* a = new PolicyAnnotation(); a->strictness="high"; return a; }());

    auto sr = saveSidecarAST(ws, "test.wh", mod);
    check(sr.success && sr.annotationCount == 5, "sidecar save 5 annotations");

    auto* mod2 = buildTestAST();
    auto lr = loadSidecarAST(ws, "test.wh", mod2);
    check(lr.success && lr.mergedCount == 5, "sidecar load restores 5 annotations");

    delete mod; delete mod2;
    fs::remove_all(ws);
}

// Test 7: setSemanticAnnotation RPC works for all new types
static void test_rpc_new_types() {
    HeadlessEditorState state;
    state.openBuffer("test.wh", "", "python");
    auto* mod = buildTestAST();
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));
    state.setAgentRole("s1", AgentRole::Refactor);

    std::vector<std::pair<std::string, json>> tests = {
        {"intrinsic", {{"instruction","x"},{"arch","x"}}},
        {"raw", {{"language","c"},{"code","x"}}},
        {"callingConv", {{"convention","cdecl"}}},
        {"link", {{"symbolName","x"},{"library","x"}}},
        {"shim", {{"strategy","vtable"}}},
        {"pointerArithmetic", json::object()},
        {"opaque", {{"reason","x"}}},
        {"target", {{"platform","linux"},{"arch","x86"}}},
        {"feature", {{"flag","SSE"},{"enabled",true}}},
        {"original", {{"sourceCode","x"},{"sourceLanguage","py"}}},
        {"mapping", {{"history",json::array({"a","b"})}}},
        {"tailCall", json::object()},
        {"loop", {{"hint","unroll"},{"factor",4}}},
        {"dataHint", {{"hint","prefetch"}}},
        {"align", {{"bytes",16}}},
        {"pack", json::object()},
        {"boundsCheck", {{"enabled",false}}},
        {"overflow", {{"behavior","wrap"}}},
        {"meta", {{"state","quoted"},{"phase","compile"}}},
        {"symbolMode", {{"mode","gensym"}}},
        {"evaluate", {{"phase","runtime"}}},
        {"template", {{"specialization","trait"}}},
        {"synthetic", {{"generator","x"},{"isStructuralRisk",true}}},
        {"policy", {{"strictness","high"},{"perf","normal"}}},
        {"ambiguity", {{"intent","x"},{"options",json::array({"a"})}}},
        {"candidate", {{"inferredTypes",json::array({"i32"})}}},
        {"tradeoff", {{"reason","x"},{"safetyCost","low"}}},
        {"choice", {{"choiceId","c1"},{"options",json::array({"a","b"})}}},
        {"decision", {{"choiceId","c1"},{"selection","a"},{"author","agent"}}},
    };

    int ok = 0;
    for (const auto& [type, fields] : tests) {
        auto r = rpc(state, "setSemanticAnnotation", {
            {"nodeId","fn1"}, {"type",type}, {"fields",fields}
        });
        if (r.contains("result") && r["result"].value("success", false)) ++ok;
    }
    check(ok == (int)tests.size(),
          "all 29 new type keys accepted by setSemanticAnnotation RPC");
}

// Test 8: Annotation taxonomy completeness — all 8 subjects represented
static void test_taxonomy_completeness() {
    // Subject 1: Intent, Complexity, Risk, Contract, SemanticTag (Phase 10a)
    // Subject 2: BitWidth, Endian, Layout, Nullability, Variance, Identity, Mut, TypeState (Phase 10c)
    // Subject 3: Atomic, Sync, ThreadModel, MemoryBarrier, Exec, Blocking, Parallel, Trap, Exception, Panic (Phase 10c)
    // Subject 4: Binding, Lookup, Capture, Visibility, Namespace, Scope (Phase 10c)
    // Subject 5: Intrinsic, Raw, CallingConv, Link, Shim, PointerArithmetic, Opaque, Target, Feature, Original, Mapping
    // Subject 6: HotCold, Inline, Pure, ConstExpr, TailCall, Loop, Data, Align, Pack, BoundsCheck, Overflow
    // Subject 7: Meta, Symbol, Evaluate, Template, Synthetic
    // Subject 8: Policy, Ambiguity, Candidate, Tradeoff, Choice, Decision

    std::vector<std::string> allSemantic = {
        // Subject 1
        "IntentAnnotation", "ComplexityAnnotation", "RiskAnnotation",
        "ContractAnnotation", "SemanticTagAnnotation",
        // Subject 2
        "BitWidthAnnotation", "EndianAnnotation", "LayoutAnnotation",
        "NullabilityAnnotation", "VarianceAnnotation", "IdentityAnnotation",
        "MutAnnotation", "TypeStateAnnotation",
        // Subject 3
        "AtomicAnnotation", "SyncAnnotation", "ThreadModelAnnotation",
        "MemoryBarrierAnnotation", "ExecAnnotation", "BlockingAnnotation",
        "ParallelAnnotation", "TrapAnnotation", "ExceptionAnnotation", "PanicAnnotation",
        // Subject 4
        "BindingAnnotation", "LookupAnnotation", "CaptureAnnotation",
        "VisibilityAnnotation", "NamespaceAnnotation", "ScopeAnnotation",
        // Subject 5
        "IntrinsicAnnotation", "RawAnnotation", "CallingConvAnnotation",
        "LinkAnnotation", "ShimAnnotation", "PointerArithmeticAnnotation",
        "OpaqueAnnotation", "TargetAnnotation", "FeatureAnnotation",
        "OriginalAnnotation", "MappingAnnotation",
        // Subject 6 (new in 10d)
        "TailCallAnnotation", "LoopAnnotation", "DataAnnotation",
        "AlignAnnotation", "PackAnnotation", "BoundsCheckAnnotation",
        "OverflowAnnotation",
        // Subject 7
        "MetaAnnotation", "SymbolAnnotation", "EvaluateAnnotation",
        "TemplateAnnotation", "SyntheticAnnotation",
        // Subject 8
        "PolicyAnnotation", "AmbiguityAnnotation", "CandidateAnnotation",
        "TradeoffAnnotation", "ChoiceAnnotation", "DecisionAnnotation"
    };

    int recognized = 0;
    for (const auto& t : allSemantic) {
        if (isSemanticAnnotation(t)) ++recognized;
        ASTNode* n = createNode(t);
        if (n) { delete n; } else { recognized--; }
    }
    check(recognized == (int)allSemantic.size(),
          "all " + std::to_string(allSemantic.size()) + " annotation types recognized and creatable");
}

int main() {
    std::cout << "=== Step 283: Phase 10d Integration Tests ===\n";
    test_shim_on_ffi();
    test_original_preserves_source();
    test_optimization_compact_ast();
    test_meta_marks_generated();
    test_policy_choice_decision();
    test_sidecar_roundtrip();
    test_rpc_new_types();
    test_taxonomy_completeness();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
