// Step 294: Generator Implementations — Java/JS/Elisp (12 tests)
#include "ast/JavaGenerator.h"
#include "ast/JavaScriptGenerator.h"
#include "ast/ElispGenerator.h"
#include "SemannoFormat.h"
#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. Java emits // prefix
void test_java_prefix() {
    TEST(java_prefix);
    JavaGenerator gen;
    CHECK(gen.commentPrefix() == "// ", "wrong prefix");
    PASS();
}

// 2. JavaScript emits // prefix
void test_js_prefix() {
    TEST(js_prefix);
    JavaScriptGenerator gen;
    CHECK(gen.commentPrefix() == "// ", "wrong prefix");
    PASS();
}

// 3. Elisp emits ;; prefix
void test_elisp_prefix() {
    TEST(elisp_prefix);
    ElispGenerator gen;
    CHECK(gen.commentPrefix() == ";; ", "wrong prefix: " + gen.commentPrefix());
    PASS();
}

// 4. Java Subject 2 type annotations
void test_java_type_annotations() {
    TEST(java_type_annotations);
    JavaGenerator gen;
    auto anno = std::make_unique<NullabilityAnnotation>();
    anno->nullable = false;
    anno->strategy = "strict";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("// @semanno:nullability") != std::string::npos, "missing: " + result);
    PASS();
}

// 5. JavaScript Subject 3 concurrency annotations
void test_js_concurrency_annotations() {
    TEST(js_concurrency_annotations);
    JavaScriptGenerator gen;
    auto anno = std::make_unique<ExecAnnotation>();
    anno->mode = "async";
    anno->runtimeHint = "Promise";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("// @semanno:exec") != std::string::npos, "missing exec");
    CHECK(result.find("async") != std::string::npos, "missing mode");
    PASS();
}

// 6. Elisp Subject 4 scope annotations
void test_elisp_scope_annotations() {
    TEST(elisp_scope_annotations);
    ElispGenerator gen;
    auto anno = std::make_unique<BindingAnnotation>();
    anno->time = "dynamic";
    std::string result = gen.generate(anno.get());
    CHECK(result.find(";; @semanno:binding") != std::string::npos, "missing: " + result);
    CHECK(result.find("dynamic") != std::string::npos, "missing time");
    PASS();
}

// 7. Java Subject 5 shim annotations
void test_java_shim_annotations() {
    TEST(java_shim_annotations);
    JavaGenerator gen;
    auto anno = std::make_unique<LinkAnnotation>();
    anno->symbolName = "System.loadLibrary";
    anno->library = "native";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:link") != std::string::npos, "missing link");
    PASS();
}

// 8. JavaScript Subject 6 optimization annotations
void test_js_optimization_annotations() {
    TEST(js_optimization_annotations);
    JavaScriptGenerator gen;
    auto anno = std::make_unique<TailCallAnnotation>();
    std::string result = gen.generate(anno.get());
    CHECK(result.find("// @semanno:tailcall") != std::string::npos, "missing: " + result);
    PASS();
}

// 9. Elisp Subject 7 meta annotations
void test_elisp_meta_annotations() {
    TEST(elisp_meta_annotations);
    ElispGenerator gen;
    auto anno = std::make_unique<MetaAnnotation>();
    anno->state = "quoted";
    anno->phase = "compile";
    std::string result = gen.generate(anno.get());
    CHECK(result.find(";; @semanno:meta") != std::string::npos, "missing meta");
    PASS();
}

// 10. All 3 generators produce semantic core annotations
void test_semantic_core_all() {
    TEST(semantic_core_all);
    auto intent = std::make_unique<IntentAnnotation>();
    intent->summary = "handles request";
    intent->category = "io";

    JavaGenerator java;
    JavaScriptGenerator js;
    ElispGenerator elisp;

    CHECK(java.generate(intent.get()).find("@semanno:intent") != std::string::npos, "java");
    CHECK(js.generate(intent.get()).find("@semanno:intent") != std::string::npos, "js");
    CHECK(elisp.generate(intent.get()).find("@semanno:intent") != std::string::npos, "elisp");
    PASS();
}

// 11. Roundtrip through Elisp generator
void test_elisp_roundtrip() {
    TEST(elisp_roundtrip);
    ElispGenerator gen;
    auto anno = std::make_unique<ComplexityAnnotation>();
    anno->timeComplexity = "O(n log n)";
    anno->cognitiveComplexity = 7;
    anno->linesOfLogic = 15;

    std::string generated = gen.generate(anno.get());
    auto entry = SemannoParser::parse(generated);
    CHECK(entry.type == "complexity", "wrong type: " + entry.type);
    CHECK(entry.properties["timeComplexity"] == "O(n log n)", "wrong time");
    CHECK(entry.properties["cognitiveComplexity"] == "7", "wrong cognitive");
    PASS();
}

// 12. No "Unknown concept" for any annotation type in all 3 generators
void test_no_unknown_all() {
    TEST(no_unknown_all);
    JavaGenerator java;
    JavaScriptGenerator js;
    ElispGenerator elisp;

    // Test a sampling across all subjects
    auto bw = std::make_unique<BitWidthAnnotation>(); bw->width = 8;
    auto sync = std::make_unique<SyncAnnotation>(); sync->primitive = "spin";
    auto vis = std::make_unique<VisibilityAnnotation>(); vis->level = "public";
    auto raw = std::make_unique<RawAnnotation>(); raw->language = "c"; raw->code = "nop";
    auto loop = std::make_unique<LoopAnnotation>(); loop->hint = "fuse";
    auto eval = std::make_unique<EvaluateAnnotation>(); eval->phase = "runtime";
    auto choice = std::make_unique<ChoiceAnnotation>(); choice->choiceId = "c1";
    auto risk = std::make_unique<RiskAnnotation>(); risk->level = "low"; risk->reason = "simple";
    auto cap = std::make_unique<CapabilityRequirement>(); cap->capability = "gc"; cap->required = true;

    const ASTNode* nodes[] = {
        bw.get(), sync.get(), vis.get(), raw.get(), loop.get(),
        eval.get(), choice.get(), risk.get(), cap.get()
    };

    for (auto* n : nodes) {
        CHECK(java.generate(n).find("Unknown") == std::string::npos,
              "Java Unknown: " + n->conceptType);
        CHECK(js.generate(n).find("Unknown") == std::string::npos,
              "JS Unknown: " + n->conceptType);
        CHECK(elisp.generate(n).find("Unknown") == std::string::npos,
              "Elisp Unknown: " + n->conceptType);
    }
    PASS();
}

int main() {
    std::cout << "=== Step 294: Java/JS/Elisp Generator Tests ===\n";
    test_java_prefix();
    test_js_prefix();
    test_elisp_prefix();
    test_java_type_annotations();
    test_js_concurrency_annotations();
    test_elisp_scope_annotations();
    test_java_shim_annotations();
    test_js_optimization_annotations();
    test_elisp_meta_annotations();
    test_semantic_core_all();
    test_elisp_roundtrip();
    test_no_unknown_all();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
