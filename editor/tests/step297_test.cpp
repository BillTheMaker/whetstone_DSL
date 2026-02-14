// Step 297: Subject 2-4 Validation Tests (12 tests)
// Tests AnnotationValidatorExtended for type system (E0600-E0608),
// concurrency (E0700-E0708), and scope (E0800-E0805) rules.

#include "AnnotationValidatorExtended.h"
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

static bool hasCode(const std::vector<AnnotationValidator::Diagnostic>& diags,
                    const std::string& code) {
    for (const auto& d : diags)
        if (d.message.find("[" + code + "]") != std::string::npos) return true;
    return false;
}

// 1. BitWidth non-power-of-2 -> E0600
void test_bitwidth_invalid() {
    TEST(bitwidth_invalid_E0600);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto bw = new BitWidthAnnotation(); bw->width = 3;
    fn->addChild("annotations", bw);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0600"), "expected E0600 for width=3");
    PASS();
}

// 2. BitWidth valid (32) -> no diagnostic
void test_bitwidth_valid() {
    TEST(bitwidth_valid_no_diag);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto bw = new BitWidthAnnotation(); bw->width = 32;
    fn->addChild("annotations", bw);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(!hasCode(diags, "E0600"), "should not trigger E0600 for width=32");
    PASS();
}

// 3. Endian invalid -> E0601
void test_endian_invalid() {
    TEST(endian_invalid_E0601);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto e = new EndianAnnotation(); e->order = "middle";
    fn->addChild("annotations", e);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0601"), "expected E0601 for order=middle");
    PASS();
}

// 4. Layout invalid mode -> E0602, invalid alignment -> E0603
void test_layout_invalid() {
    TEST(layout_invalid_E0602_E0603);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto lay = new LayoutAnnotation(); lay->mode = "scattered"; lay->alignment = 3;
    fn->addChild("annotations", lay);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0602"), "expected E0602 for mode=scattered");
    CHECK(hasCode(diags, "E0603"), "expected E0603 for alignment=3");
    PASS();
}

// 5. Nullability invalid -> E0604
void test_nullability_invalid() {
    TEST(nullability_invalid_E0604);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto n = new NullabilityAnnotation(); n->strategy = "maybe";
    fn->addChild("annotations", n);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0604"), "expected E0604 for strategy=maybe");
    PASS();
}

// 6. Variance invalid -> E0605
void test_variance_invalid() {
    TEST(variance_invalid_E0605);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto va = new VarianceAnnotation(); va->variance = "bivariant";
    fn->addChild("annotations", va);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0605"), "expected E0605 for variance=bivariant");
    PASS();
}

// 7. Atomic invalid -> E0700
void test_atomic_invalid() {
    TEST(atomic_invalid_E0700);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto a = new AtomicAnnotation(); a->consistency = "strong";
    fn->addChild("annotations", a);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0700"), "expected E0700 for consistency=strong");
    PASS();
}

// 8. Exec async without ThreadModel -> E0704
void test_exec_async_no_threadmodel() {
    TEST(exec_async_no_threadmodel_E0704);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto exec = new ExecAnnotation(); exec->mode = "async";
    fn->addChild("annotations", exec);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0704"), "expected E0704 for async without ThreadModel");
    PASS();
}

// 9. Exec async WITH ThreadModel -> no E0704
void test_exec_async_with_threadmodel() {
    TEST(exec_async_with_threadmodel_no_E0704);
    auto mod = std::make_unique<Module>();
    // Add ThreadModel to parent module
    auto tm = new ThreadModelAnnotation(); tm->model = "os";
    mod->addChild("annotations", tm);

    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto exec = new ExecAnnotation(); exec->mode = "async";
    fn->addChild("annotations", exec);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(!hasCode(diags, "E0704"), "should not trigger E0704 when ThreadModel is on ancestor");
    PASS();
}

// 10. Capture invalid -> E0802
void test_capture_invalid() {
    TEST(capture_invalid_E0802);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto c = new CaptureAnnotation(); c->strategy = "borrow";
    fn->addChild("annotations", c);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0802"), "expected E0802 for strategy=borrow");
    PASS();
}

// 11. Visibility invalid -> E0803
void test_visibility_invalid() {
    TEST(visibility_invalid_E0803);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto vis = new VisibilityAnnotation(); vis->level = "protected";
    fn->addChild("annotations", vis);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0803"), "expected E0803 for level=protected");
    PASS();
}

// 12. All valid Subject 2-4 annotations -> 0 diagnostics
void test_all_valid_subject_2_4() {
    TEST(all_valid_subject_2_4_no_diags);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";

    // Valid Subject 2
    auto bw = new BitWidthAnnotation(); bw->width = 64;
    fn->addChild("annotations", bw);
    auto en = new EndianAnnotation(); en->order = "big";
    fn->addChild("annotations", en);
    auto lay = new LayoutAnnotation(); lay->mode = "packed"; lay->alignment = 0;
    fn->addChild("annotations", lay);
    auto nu = new NullabilityAnnotation(); nu->strategy = "strict";
    fn->addChild("annotations", nu);
    auto va = new VarianceAnnotation(); va->variance = "covariant";
    fn->addChild("annotations", va);

    // Valid Subject 3
    auto at = new AtomicAnnotation(); at->consistency = "seq_cst";
    fn->addChild("annotations", at);
    auto sy = new SyncAnnotation(); sy->primitive = "monitor";
    fn->addChild("annotations", sy);

    // Valid Subject 4
    auto bi = new BindingAnnotation(); bi->time = "static";
    fn->addChild("annotations", bi);
    auto ca = new CaptureAnnotation(); ca->strategy = "value";
    fn->addChild("annotations", ca);
    auto vis = new VisibilityAnnotation(); vis->level = "public";
    fn->addChild("annotations", vis);

    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(diags.empty(), "expected 0 diagnostics for all valid annotations, got " + std::to_string(diags.size()));
    PASS();
}

int main() {
    std::cout << "=== Step 297: Subject 2-4 Validation Tests ===\n";
    test_bitwidth_invalid();
    test_bitwidth_valid();
    test_endian_invalid();
    test_layout_invalid();
    test_nullability_invalid();
    test_variance_invalid();
    test_atomic_invalid();
    test_exec_async_no_threadmodel();
    test_exec_async_with_threadmodel();
    test_capture_invalid();
    test_visibility_invalid();
    test_all_valid_subject_2_4();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
