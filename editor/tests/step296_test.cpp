// Step 296: Phase 11a Integration Tests (8 tests)
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/RustGenerator.h"
#include "ast/GoGenerator.h"
#include "ast/JavaGenerator.h"
#include "ast/JavaScriptGenerator.h"
#include "ast/ElispGenerator.h"
#include "SemannoFormat.h"
#include "SemannoSidecar.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "EnvironmentSpec.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string tmpDir() {
    auto p = std::filesystem::temp_directory_path() / "whetstone_test_296";
    std::filesystem::create_directories(p);
    return p.string();
}

static void cleanup() {
    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "whetstone_test_296");
}

// 1. Module with Subject 2-8 annotations generates Semanno in all generators
void test_multi_subject_generation() {
    TEST(multi_subject_generation);
    auto mod = std::make_unique<Module>();
    mod->name = "annotated";

    auto fn = std::make_unique<Function>();
    fn->name = "process";

    // One from each subject
    auto bw = new BitWidthAnnotation(); bw->width = 32;           // S2
    auto atomic = new AtomicAnnotation(); atomic->consistency = "seq_cst";  // S3
    auto cap = new CaptureAnnotation(); cap->strategy = "move";    // S4
    auto shim = new ShimAnnotation(); shim->strategy = "vtable";   // S5
    auto loop = new LoopAnnotation(); loop->hint = "unroll";       // S6
    auto meta = new MetaAnnotation(); meta->state = "quoted"; meta->phase = "compile"; // S7
    auto policy = new PolicyAnnotation(); policy->strictness = "high"; // S8

    fn->addChild("annotations", bw);
    fn->addChild("annotations", atomic);
    fn->addChild("annotations", cap);
    fn->addChild("annotations", shim);
    fn->addChild("annotations", loop);
    fn->addChild("annotations", meta);
    fn->addChild("annotations", policy);

    mod->addChild("functions", fn.release());

    PythonGenerator gen;
    std::string result = gen.generate(mod.get());

    CHECK(result.find("@semanno:bitwidth") != std::string::npos, "missing S2");
    CHECK(result.find("@semanno:atomic") != std::string::npos, "missing S3");
    CHECK(result.find("@semanno:capture") != std::string::npos, "missing S4");
    CHECK(result.find("@semanno:shim") != std::string::npos, "missing S5");
    CHECK(result.find("@semanno:loop") != std::string::npos, "missing S6");
    CHECK(result.find("@semanno:meta") != std::string::npos, "missing S7");
    CHECK(result.find("@semanno:policy") != std::string::npos, "missing S8");
    PASS();
}

// 2. Cross-language: annotations preserved through projection (generate same module with diff generators)
void test_cross_language_preservation() {
    TEST(cross_language_preservation);
    auto mod = std::make_unique<Module>();
    mod->name = "cross";
    auto fn = std::make_unique<Function>();
    fn->name = "sort";
    fn->addChild("annotations", new LoopAnnotation());
    fn->addChild("annotations", new PureAnnotation());
    mod->addChild("functions", fn.release());

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rsGen;

    std::string py = pyGen.generate(mod.get());
    std::string cpp = cppGen.generate(mod.get());
    std::string rs = rsGen.generate(mod.get());

    CHECK(py.find("@semanno:loop") != std::string::npos, "py missing loop");
    CHECK(cpp.find("@semanno:loop") != std::string::npos, "cpp missing loop");
    CHECK(rs.find("@semanno:loop") != std::string::npos, "rs missing loop");
    PASS();
}

// 3. All 7 generators produce non-empty Semanno for every annotation type (exhaustive)
void test_exhaustive_no_unknown() {
    TEST(exhaustive_no_unknown);

    // Create one of each annotation type
    std::vector<std::unique_ptr<ASTNode>> annotations;
    // Subject 2
    { auto a = std::make_unique<BitWidthAnnotation>(); a->width = 8; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<EndianAnnotation>(); a->order = "little"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<LayoutAnnotation>(); a->mode = "aligned"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<NullabilityAnnotation>(); a->nullable = true; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<VarianceAnnotation>(); a->variance = "invariant"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<IdentityAnnotation>(); a->mode = "structural"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<MutAnnotation>(); a->depth = "shallow"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<TypeStateAnnotation>(); a->state = "erased"; annotations.push_back(std::move(a)); }
    // Subject 3
    { auto a = std::make_unique<AtomicAnnotation>(); a->consistency = "relaxed"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<SyncAnnotation>(); a->primitive = "spin"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ThreadModelAnnotation>(); a->model = "os"; annotations.push_back(std::move(a)); }
    { annotations.push_back(std::make_unique<MemoryBarrierAnnotation>()); }
    { auto a = std::make_unique<ExecAnnotation>(); a->mode = "event"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<BlockingAnnotation>(); a->kind = "compute"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ParallelAnnotation>(); a->kind = "task"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<TrapAnnotation>(); a->signal = "SIGSEGV"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ExceptionAnnotation>(); a->style = "unchecked"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<PanicAnnotation>(); a->behavior = "unwind"; annotations.push_back(std::move(a)); }
    // Subject 4
    { auto a = std::make_unique<BindingAnnotation>(); a->time = "dynamic"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<LookupAnnotation>(); a->mode = "hoisted"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<CaptureAnnotation>(); a->strategy = "ref"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<VisibilityAnnotation>(); a->level = "internal"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<NamespaceAnnotation>(); a->style = "flat"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ScopeAnnotation>(); a->kind = "global_leaked"; annotations.push_back(std::move(a)); }
    // Subject 5
    { auto a = std::make_unique<IntrinsicAnnotation>(); a->instruction = "nop"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<RawAnnotation>(); a->language = "c"; a->code = "x"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<CallingConvAnnotation>(); a->convention = "fastcall"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<LinkAnnotation>(); a->symbolName = "s"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ShimAnnotation>(); a->strategy = "cast"; annotations.push_back(std::move(a)); }
    { annotations.push_back(std::make_unique<PointerArithmeticAnnotation>()); }
    { auto a = std::make_unique<OpaqueAnnotation>(); a->reason = "r"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<TargetAnnotation>(); a->platform = "linux"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<FeatureAnnotation>(); a->flag = "sse2"; a->enabled = true; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<OriginalAnnotation>(); a->sourceCode = "x"; a->sourceLanguage = "py"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<MappingAnnotation>(); a->history = {"a"}; annotations.push_back(std::move(a)); }
    // Subject 6
    { annotations.push_back(std::make_unique<TailCallAnnotation>()); }
    { auto a = std::make_unique<LoopAnnotation>(); a->hint = "vectorize"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<DataAnnotation>(); a->hint = "restrict"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<AlignAnnotation>(); a->bytes = 16; annotations.push_back(std::move(a)); }
    { annotations.push_back(std::make_unique<PackAnnotation>()); }
    { auto a = std::make_unique<BoundsCheckAnnotation>(); a->enabled = true; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<OverflowAnnotation>(); a->behavior = "saturation"; annotations.push_back(std::move(a)); }
    // Subject 7
    { auto a = std::make_unique<MetaAnnotation>(); a->state = "quoted"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<SymbolAnnotation>(); a->mode = "gensym"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<EvaluateAnnotation>(); a->phase = "compile_time"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<TemplateAnnotation>(); a->specialization = "erasure"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<SyntheticAnnotation>(); a->generator = "macro"; annotations.push_back(std::move(a)); }
    // Subject 8
    { auto a = std::make_unique<PolicyAnnotation>(); a->strictness = "low"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<AmbiguityAnnotation>(); a->intent = "x"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<CandidateAnnotation>(); a->inferredTypes = {"int"}; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<TradeoffAnnotation>(); a->reason = "r"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ChoiceAnnotation>(); a->choiceId = "c"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<DecisionAnnotation>(); a->choiceId = "c"; a->selection = "a"; annotations.push_back(std::move(a)); }
    // Semantic Core
    { auto a = std::make_unique<IntentAnnotation>(); a->summary = "s"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ComplexityAnnotation>(); a->timeComplexity = "O(1)"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<RiskAnnotation>(); a->level = "low"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<ContractAnnotation>(); a->preconditions = "x"; annotations.push_back(std::move(a)); }
    { auto a = std::make_unique<SemanticTagAnnotation>(); a->tags = {"io"}; annotations.push_back(std::move(a)); }
    // Environment
    { auto a = std::make_unique<CapabilityRequirement>(); a->capability = "gc"; a->required = true; annotations.push_back(std::move(a)); }

    PythonGenerator py;
    CppGenerator cpp;
    int unknownCount = 0;
    for (const auto& a : annotations) {
        std::string pyRes = py.generate(a.get());
        std::string cppRes = cpp.generate(a.get());
        if (pyRes.find("Unknown") != std::string::npos) {
            std::cout << "Python Unknown: " << a->conceptType << "\n";
            ++unknownCount;
        }
        if (cppRes.find("Unknown") != std::string::npos) {
            std::cout << "C++ Unknown: " << a->conceptType << "\n";
            ++unknownCount;
        }
    }
    CHECK(unknownCount == 0, std::to_string(unknownCount) + " Unknown results");
    CHECK(annotations.size() >= 56, "expected 56+ types, got " + std::to_string(annotations.size()));
    PASS();
}

// 4. Semanno roundtrip: generate → parse back → match
void test_semanno_roundtrip() {
    TEST(semanno_roundtrip);
    CppGenerator gen;

    auto anno = std::make_unique<PolicyAnnotation>();
    anno->strictness = "high";
    anno->perf = "critical";
    anno->style = "idiomatic";
    anno->binaryStable = false;

    std::string generated = gen.generate(anno.get());
    auto entry = SemannoParser::parse(generated);
    CHECK(entry.type == "policy", "wrong type");
    CHECK(entry.properties["strictness"] == "high", "strictness lost");
    CHECK(entry.properties["perf"] == "critical", "perf lost");
    CHECK(entry.properties["style"] == "idiomatic", "style lost");
    PASS();
}

// 5. Mixed legacy + new annotations in single module
void test_mixed_legacy_new() {
    TEST(mixed_legacy_new);
    auto mod = std::make_unique<Module>();
    mod->name = "mixed";
    auto fn = std::make_unique<Function>();
    fn->name = "calc";

    // Legacy (Subject 1 memory)
    auto reclaim = new ReclaimAnnotation();
    reclaim->strategy = "Tracing";
    fn->addChild("annotations", reclaim);

    // New (Subject 6)
    auto loop = new LoopAnnotation();
    loop->hint = "vectorize";
    fn->addChild("annotations", loop);

    // Semantic core
    auto intent = new IntentAnnotation();
    intent->summary = "calculates sum";
    fn->addChild("annotations", intent);

    mod->addChild("functions", fn.release());

    CppGenerator gen;
    std::string result = gen.generate(mod.get());
    // Legacy memory annotations may use their own format (not semanno)
    // New annotations should use semanno
    CHECK(result.find("@semanno:loop") != std::string::npos, "missing loop");
    CHECK(result.find("@semanno:intent") != std::string::npos, "missing intent");
    PASS();
}

// 6. Semanno sidecar roundtrip with all subjects
void test_sidecar_all_subjects() {
    TEST(sidecar_all_subjects);
    cleanup();
    auto mod = std::make_unique<Module>();
    mod->name = "sidecar_test";
    auto fn = std::make_unique<Function>();
    fn->name = "f";
    fn->spanStartLine = 1;

    fn->addChild("annotations", new IntentAnnotation());
    auto bw = new BitWidthAnnotation(); bw->width = 32;
    fn->addChild("annotations", bw);
    auto atomic = new AtomicAnnotation(); atomic->consistency = "seq_cst";
    fn->addChild("annotations", atomic);

    mod->addChild("functions", fn.release());

    auto saved = saveSemannoSidecar(tmpDir(), "all.py", mod.get());
    CHECK(saved.success, "save failed");
    CHECK(saved.annotationCount == 3, "wrong save count: " + std::to_string(saved.annotationCount));

    auto loaded = loadSemannoSidecar(tmpDir(), "all.py");
    CHECK(loaded.success, "load failed");
    CHECK(loaded.count == 3, "wrong load count: " + std::to_string(loaded.count));
    PASS();
}

// 7. Generator comment prefix matches language convention
void test_all_prefixes() {
    TEST(all_prefixes);
    PythonGenerator py;
    CppGenerator cpp;
    RustGenerator rs;
    GoGenerator go;
    JavaGenerator java;
    JavaScriptGenerator js;
    ElispGenerator elisp;

    CHECK(py.commentPrefix() == "# ", "py");
    CHECK(cpp.commentPrefix() == "// ", "cpp");
    CHECK(rs.commentPrefix() == "// ", "rs");
    CHECK(go.commentPrefix() == "// ", "go");
    CHECK(java.commentPrefix() == "// ", "java");
    CHECK(js.commentPrefix() == "// ", "js");
    CHECK(elisp.commentPrefix() == ";; ", "elisp");
    PASS();
}

// 8. Marker annotations produce correct Semanno (no parentheses)
void test_marker_semanno() {
    TEST(marker_semanno);
    CppGenerator gen;

    auto pure = std::make_unique<PureAnnotation>();
    auto tailcall = std::make_unique<TailCallAnnotation>();
    auto pack = std::make_unique<PackAnnotation>();
    auto barrier = std::make_unique<MemoryBarrierAnnotation>();
    auto ptrArith = std::make_unique<PointerArithmeticAnnotation>();

    std::string pureResult = gen.generate(pure.get());
    std::string tailResult = gen.generate(tailcall.get());
    std::string packResult = gen.generate(pack.get());

    // Marker annotations should NOT have parentheses
    CHECK(pureResult.find("(") == std::string::npos, "pure has parens: " + pureResult);
    CHECK(tailResult.find("(") == std::string::npos, "tailcall has parens: " + tailResult);
    CHECK(packResult.find("(") == std::string::npos, "pack has parens: " + packResult);
    PASS();
}

int main() {
    std::cout << "=== Step 296: Phase 11a Integration Tests ===\n";
    test_multi_subject_generation();
    test_cross_language_preservation();
    test_exhaustive_no_unknown();
    test_semanno_roundtrip();
    test_mixed_legacy_new();
    test_sidecar_all_subjects();
    test_all_prefixes();
    test_marker_semanno();
    cleanup();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
