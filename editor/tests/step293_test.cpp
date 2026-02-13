// Step 293: Generator Implementations — Python/C++/Rust/Go (12 tests)
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/RustGenerator.h"
#include "ast/GoGenerator.h"
#include "SemannoFormat.h"
#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. Python emits # prefix for Subject 2
void test_python_type_annotations() {
    TEST(python_type_annotations);
    PythonGenerator gen;
    auto anno = std::make_unique<BitWidthAnnotation>();
    anno->width = 64;
    std::string result = gen.generate(anno.get());
    CHECK(result.find("# @semanno:bitwidth") != std::string::npos, "wrong prefix: " + result);
    PASS();
}

// 2. C++ emits // prefix for Subject 3
void test_cpp_concurrency_annotations() {
    TEST(cpp_concurrency_annotations);
    CppGenerator gen;
    auto anno = std::make_unique<AtomicAnnotation>();
    anno->consistency = "seq_cst";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("// @semanno:atomic") != std::string::npos, "wrong prefix: " + result);
    PASS();
}

// 3. Rust emits // prefix for Subject 4
void test_rust_scope_annotations() {
    TEST(rust_scope_annotations);
    RustGenerator gen;
    auto anno = std::make_unique<CaptureAnnotation>();
    anno->strategy = "move";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("// @semanno:capture") != std::string::npos, "wrong prefix: " + result);
    PASS();
}

// 4. Go emits // prefix for Subject 5
void test_go_shim_annotations() {
    TEST(go_shim_annotations);
    GoGenerator gen;
    auto anno = std::make_unique<CallingConvAnnotation>();
    anno->convention = "cdecl";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("// @semanno:callingconv") != std::string::npos, "wrong prefix: " + result);
    PASS();
}

// 5. Python with mixed old + new annotations in module
void test_python_mixed_annotations() {
    TEST(python_mixed_annotations);
    PythonGenerator gen;

    auto mod = std::make_unique<Module>();
    mod->name = "test";
    auto fn = std::make_unique<Function>();
    fn->name = "compute";

    auto oldAnno = std::make_unique<PureAnnotation>();
    fn->addChild("annotations", oldAnno.release());

    auto newAnno = std::make_unique<LoopAnnotation>();
    newAnno->hint = "vectorize";
    newAnno->factor = 4;
    fn->addChild("annotations", newAnno.release());

    mod->addChild("functions", fn.release());

    std::string result = gen.generate(mod.get());
    CHECK(result.find("@semanno:loop") != std::string::npos, "missing new annotation");
    PASS();
}

// 6. C++ generates all Subject 6 types
void test_cpp_optimization_annotations() {
    TEST(cpp_optimization_annotations);
    CppGenerator gen;

    auto tailcall = std::make_unique<TailCallAnnotation>();
    auto align = std::make_unique<AlignAnnotation>(); align->bytes = 32;
    auto pack = std::make_unique<PackAnnotation>();
    auto bc = std::make_unique<BoundsCheckAnnotation>(); bc->enabled = true;

    CHECK(gen.generate(tailcall.get()).find("@semanno:tailcall") != std::string::npos, "tailcall");
    CHECK(gen.generate(align.get()).find("@semanno:align") != std::string::npos, "align");
    CHECK(gen.generate(pack.get()).find("@semanno:pack") != std::string::npos, "pack");
    CHECK(gen.generate(bc.get()).find("@semanno:boundscheck") != std::string::npos, "boundscheck");
    PASS();
}

// 7. Rust generates Subject 7 types
void test_rust_meta_annotations() {
    TEST(rust_meta_annotations);
    RustGenerator gen;

    auto meta = std::make_unique<MetaAnnotation>();
    meta->state = "quoted";
    meta->phase = "compile";
    auto sym = std::make_unique<SymbolAnnotation>();
    sym->mode = "interned";

    CHECK(gen.generate(meta.get()).find("@semanno:meta") != std::string::npos, "meta");
    CHECK(gen.generate(sym.get()).find("@semanno:symbol") != std::string::npos, "symbol");
    PASS();
}

// 8. Go generates Subject 8 types
void test_go_policy_annotations() {
    TEST(go_policy_annotations);
    GoGenerator gen;

    auto policy = std::make_unique<PolicyAnnotation>();
    policy->strictness = "low";
    auto decision = std::make_unique<DecisionAnnotation>();
    decision->choiceId = "c1";
    decision->selection = "optionA";

    CHECK(gen.generate(policy.get()).find("@semanno:policy") != std::string::npos, "policy");
    CHECK(gen.generate(decision.get()).find("@semanno:decision") != std::string::npos, "decision");
    PASS();
}

// 9. Semantic core annotations across all 4 generators
void test_semantic_core_all_generators() {
    TEST(semantic_core_all_generators);
    auto intent = std::make_unique<IntentAnnotation>();
    intent->summary = "computes sum";
    intent->category = "pure";

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rsGen;
    GoGenerator goGen;

    CHECK(pyGen.generate(intent.get()).find("@semanno:intent") != std::string::npos, "py intent");
    CHECK(cppGen.generate(intent.get()).find("@semanno:intent") != std::string::npos, "cpp intent");
    CHECK(rsGen.generate(intent.get()).find("@semanno:intent") != std::string::npos, "rs intent");
    CHECK(goGen.generate(intent.get()).find("@semanno:intent") != std::string::npos, "go intent");
    PASS();
}

// 10. CapabilityRequirement across generators
void test_capability_all_generators() {
    TEST(capability_all_generators);
    auto cap = std::make_unique<CapabilityRequirement>();
    cap->capability = "io.fs";
    cap->required = true;

    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rsGen;
    GoGenerator goGen;

    CHECK(pyGen.generate(cap.get()).find("@semanno:capability") != std::string::npos, "py cap");
    CHECK(cppGen.generate(cap.get()).find("@semanno:capability") != std::string::npos, "cpp cap");
    CHECK(rsGen.generate(cap.get()).find("@semanno:capability") != std::string::npos, "rs cap");
    CHECK(goGen.generate(cap.get()).find("@semanno:capability") != std::string::npos, "go cap");
    PASS();
}

// 11. commentPrefix() returns correct string
void test_comment_prefix() {
    TEST(comment_prefix);
    PythonGenerator pyGen;
    CppGenerator cppGen;
    RustGenerator rsGen;
    GoGenerator goGen;

    CHECK(pyGen.commentPrefix() == "# ", "py prefix");
    CHECK(cppGen.commentPrefix() == "// ", "cpp prefix");
    CHECK(rsGen.commentPrefix() == "// ", "rs prefix");
    CHECK(goGen.commentPrefix() == "// ", "go prefix");
    PASS();
}

// 12. Roundtrip: emit from generator, parse back
void test_generator_roundtrip() {
    TEST(generator_roundtrip);
    CppGenerator gen;
    auto anno = std::make_unique<LoopAnnotation>();
    anno->hint = "vectorize";
    anno->factor = 4;

    std::string generated = gen.generate(anno.get());
    // Generated should be "// @semanno:loop(hint="vectorize",factor=4)"
    auto entry = SemannoParser::parse(generated);
    CHECK(entry.type == "loop", "wrong type: " + entry.type);
    CHECK(entry.properties["hint"] == "vectorize", "wrong hint");
    CHECK(entry.properties["factor"] == "4", "wrong factor");
    PASS();
}

int main() {
    std::cout << "=== Step 293: Python/C++/Rust/Go Generator Tests ===\n";
    test_python_type_annotations();
    test_cpp_concurrency_annotations();
    test_rust_scope_annotations();
    test_go_shim_annotations();
    test_python_mixed_annotations();
    test_cpp_optimization_annotations();
    test_rust_meta_annotations();
    test_go_policy_annotations();
    test_semantic_core_all_generators();
    test_capability_all_generators();
    test_comment_prefix();
    test_generator_roundtrip();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
