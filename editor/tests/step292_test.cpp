// Step 292: ProjectionGenerator — Subject 5-8 + Semantic Visitors (12 tests)
#include "ast/ProjectionGenerator.h"
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "SemannoFormat.h"
#include <iostream>
#include <cassert>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. IntrinsicAnnotation (Subject 5)
void test_intrinsic_dispatch() {
    TEST(intrinsic_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<IntrinsicAnnotation>();
    anno->instruction = "_mm_load_ps";
    anno->arch = "sse";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:intrinsic") != std::string::npos, "missing semanno");
    PASS();
}

// 2. CallingConvAnnotation
void test_callingconv_dispatch() {
    TEST(callingconv_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<CallingConvAnnotation>();
    anno->convention = "stdcall";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:callingconv") != std::string::npos, "missing semanno");
    PASS();
}

// 3. TargetAnnotation
void test_target_dispatch() {
    TEST(target_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<TargetAnnotation>();
    anno->platform = "windows";
    anno->arch = "x86_64";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:target") != std::string::npos, "missing semanno");
    CHECK(result.find("windows") != std::string::npos, "missing platform");
    PASS();
}

// 4. TailCallAnnotation (Subject 6 marker)
void test_tailcall_dispatch() {
    TEST(tailcall_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<TailCallAnnotation>();
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:tailcall") != std::string::npos, "missing semanno");
    CHECK(result.find("Unknown") == std::string::npos, "returned Unknown");
    PASS();
}

// 5. LoopAnnotation
void test_loop_dispatch() {
    TEST(loop_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<LoopAnnotation>();
    anno->hint = "unroll";
    anno->factor = 8;
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:loop") != std::string::npos, "missing semanno");
    PASS();
}

// 6. MetaAnnotation (Subject 7)
void test_meta_dispatch() {
    TEST(meta_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<MetaAnnotation>();
    anno->state = "unquoted";
    anno->phase = "runtime";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:meta") != std::string::npos, "missing semanno");
    PASS();
}

// 7. SyntheticAnnotation
void test_synthetic_dispatch() {
    TEST(synthetic_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<SyntheticAnnotation>();
    anno->generator = "macro_expand";
    anno->isStructuralRisk = true;
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:synthetic") != std::string::npos, "missing semanno");
    CHECK(result.find("macro_expand") != std::string::npos, "missing generator");
    PASS();
}

// 8. PolicyAnnotation (Subject 8)
void test_policy_dispatch() {
    TEST(policy_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<PolicyAnnotation>();
    anno->strictness = "high";
    anno->perf = "normal";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:policy") != std::string::npos, "missing semanno");
    PASS();
}

// 9. IntentAnnotation (Semantic Core)
void test_intent_dispatch() {
    TEST(intent_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<IntentAnnotation>();
    anno->summary = "sorts data in-place";
    anno->category = "mutation";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:intent") != std::string::npos, "missing semanno");
    PASS();
}

// 10. CapabilityRequirement (Environment)
void test_capability_dispatch() {
    TEST(capability_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<CapabilityRequirement>();
    anno->capability = "io.net";
    anno->required = true;
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:capability") != std::string::npos, "missing semanno");
    PASS();
}

// 11. Exhaustive: no "Unknown" for any Subject 5-8 type
void test_no_unknown_subject5_8() {
    TEST(no_unknown_subject5_8);
    PythonGenerator gen;

    auto raw = std::make_unique<RawAnnotation>(); raw->language = "c"; raw->code = "asm";
    auto link = std::make_unique<LinkAnnotation>(); link->symbolName = "foo"; link->library = "bar";
    auto shim = std::make_unique<ShimAnnotation>(); shim->strategy = "vtable";
    auto pa = std::make_unique<PointerArithmeticAnnotation>();
    auto opq = std::make_unique<OpaqueAnnotation>(); opq->reason = "opaque";
    auto feat = std::make_unique<FeatureAnnotation>(); feat->flag = "avx2"; feat->enabled = true;
    auto orig = std::make_unique<OriginalAnnotation>(); orig->sourceCode = "x"; orig->sourceLanguage = "py";
    auto map = std::make_unique<MappingAnnotation>(); map->history = {"step1", "step2"};

    auto data = std::make_unique<DataAnnotation>(); data->hint = "prefetch";
    auto align = std::make_unique<AlignAnnotation>(); align->bytes = 16;
    auto pack = std::make_unique<PackAnnotation>();
    auto bc = std::make_unique<BoundsCheckAnnotation>(); bc->enabled = false;
    auto ov = std::make_unique<OverflowAnnotation>(); ov->behavior = "wrap";

    auto sym = std::make_unique<SymbolAnnotation>(); sym->mode = "gensym";
    auto eval = std::make_unique<EvaluateAnnotation>(); eval->phase = "compile_time";
    auto tmpl = std::make_unique<TemplateAnnotation>(); tmpl->specialization = "monomorphize";

    auto amb = std::make_unique<AmbiguityAnnotation>(); amb->intent = "x";
    auto cand = std::make_unique<CandidateAnnotation>(); cand->inferredTypes = {"int"};
    auto trade = std::make_unique<TradeoffAnnotation>(); trade->reason = "r";
    auto choice = std::make_unique<ChoiceAnnotation>(); choice->choiceId = "c1";
    auto dec = std::make_unique<DecisionAnnotation>(); dec->choiceId = "c1"; dec->selection = "a";

    const ASTNode* allNodes[] = {
        raw.get(), link.get(), shim.get(), pa.get(), opq.get(),
        feat.get(), orig.get(), map.get(), data.get(), align.get(),
        pack.get(), bc.get(), ov.get(), sym.get(), eval.get(),
        tmpl.get(), amb.get(), cand.get(), trade.get(), choice.get(), dec.get()
    };
    for (auto* n : allNodes) {
        std::string result = gen.generate(n);
        CHECK(result.find("Unknown") == std::string::npos,
              "Unknown concept for " + n->conceptType + ": " + result);
    }
    PASS();
}

// 12. Exhaustive: no "Unknown" for Semantic Core + Environment
void test_no_unknown_semantic_env() {
    TEST(no_unknown_semantic_env);
    CppGenerator gen;

    auto intent = std::make_unique<IntentAnnotation>(); intent->summary = "s"; intent->category = "c";
    auto complex = std::make_unique<ComplexityAnnotation>(); complex->timeComplexity = "O(n)";
    auto risk = std::make_unique<RiskAnnotation>(); risk->level = "high"; risk->reason = "r";
    auto contract = std::make_unique<ContractAnnotation>(); contract->preconditions = "x>0";
    auto tags = std::make_unique<SemanticTagAnnotation>(); tags->tags = {"io", "crypto"};
    auto cap = std::make_unique<CapabilityRequirement>(); cap->capability = "threads"; cap->required = true;

    const ASTNode* allNodes[] = {
        intent.get(), complex.get(), risk.get(), contract.get(), tags.get(), cap.get()
    };
    for (auto* n : allNodes) {
        std::string result = gen.generate(n);
        CHECK(result.find("Unknown") == std::string::npos,
              "Unknown concept for " + n->conceptType);
    }
    PASS();
}

int main() {
    std::cout << "=== Step 292: Subject 5-8 + Semantic Visitor Tests ===\n";
    test_intrinsic_dispatch();
    test_callingconv_dispatch();
    test_target_dispatch();
    test_tailcall_dispatch();
    test_loop_dispatch();
    test_meta_dispatch();
    test_synthetic_dispatch();
    test_policy_dispatch();
    test_intent_dispatch();
    test_capability_dispatch();
    test_no_unknown_subject5_8();
    test_no_unknown_semantic_env();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
