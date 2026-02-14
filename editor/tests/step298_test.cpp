// Step 298: Subject 5-8 Validation Tests (12 tests)
// Tests AnnotationValidatorExtended for FFI (E0900-E0901), optimization
// (E1000-E1004), meta-programming (E1100-E1104), and policy (E1200-E1202).

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

// 1. CallingConv invalid -> E0900
void test_callingconv_invalid() {
    TEST(callingconv_invalid_E0900);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto cc = new CallingConvAnnotation(); cc->convention = "thiscall";
    fn->addChild("annotations", cc);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0900"), "expected E0900 for convention=thiscall");
    PASS();
}

// 2. Shim invalid -> E0901
void test_shim_invalid() {
    TEST(shim_invalid_E0901);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto s = new ShimAnnotation(); s->strategy = "proxy";
    fn->addChild("annotations", s);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E0901"), "expected E0901 for strategy=proxy");
    PASS();
}

// 3. Inline(Always) + TailCall -> E1000
void test_inline_always_tailcall() {
    TEST(inline_always_tailcall_E1000);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto inl = new InlineAnnotation(); inl->mode = "Always";
    fn->addChild("annotations", inl);
    fn->addChild("annotations", new TailCallAnnotation());
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1000"), "expected E1000 for Inline(Always)+TailCall");
    PASS();
}

// 4. Inline(Hint) + TailCall -> no E1000
void test_inline_hint_tailcall() {
    TEST(inline_hint_tailcall_no_E1000);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto inl = new InlineAnnotation(); inl->mode = "Hint";
    fn->addChild("annotations", inl);
    fn->addChild("annotations", new TailCallAnnotation());
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(!hasCode(diags, "E1000"), "should not trigger E1000 for Hint mode");
    PASS();
}

// 5. Loop invalid hint -> E1001
void test_loop_invalid_hint() {
    TEST(loop_invalid_hint_E1001);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto lo = new LoopAnnotation(); lo->hint = "parallelize";
    fn->addChild("annotations", lo);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1001"), "expected E1001 for hint=parallelize");
    PASS();
}

// 6. Loop unroll factor=0 -> E1002
void test_loop_unroll_factor_zero() {
    TEST(loop_unroll_factor_zero_E1002);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto lo = new LoopAnnotation(); lo->hint = "unroll"; lo->factor = 0;
    fn->addChild("annotations", lo);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1002"), "expected E1002 for unroll with factor=0");
    PASS();
}

// 7. Align non-power-of-2 -> E1003
void test_align_invalid() {
    TEST(align_invalid_E1003);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto al = new AlignAnnotation(); al->bytes = 3;
    fn->addChild("annotations", al);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1003"), "expected E1003 for bytes=3");
    PASS();
}

// 8. Overflow invalid -> E1004
void test_overflow_invalid() {
    TEST(overflow_invalid_E1004);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto ov = new OverflowAnnotation(); ov->behavior = "ignore";
    fn->addChild("annotations", ov);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1004"), "expected E1004 for behavior=ignore");
    PASS();
}

// 9. Meta invalid state -> E1100
void test_meta_invalid_state() {
    TEST(meta_invalid_state_E1100);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto m = new MetaAnnotation(); m->state = "mixed"; m->phase = "compile";
    fn->addChild("annotations", m);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1100"), "expected E1100 for state=mixed");
    PASS();
}

// 10. Template invalid specialization -> E1104
void test_template_invalid() {
    TEST(template_invalid_E1104);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto t = new TemplateAnnotation(); t->specialization = "vtable";
    fn->addChild("annotations", t);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1104"), "expected E1104 for specialization=vtable");
    PASS();
}

// 11. Policy invalid strictness -> E1200
void test_policy_invalid_strictness() {
    TEST(policy_invalid_strictness_E1200);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";
    auto p = new PolicyAnnotation(); p->strictness = "medium";
    fn->addChild("annotations", p);
    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(hasCode(diags, "E1200"), "expected E1200 for strictness=medium");
    PASS();
}

// 12. All valid Subject 5-8 annotations -> 0 diagnostics
void test_all_valid_subject_5_8() {
    TEST(all_valid_subject_5_8_no_diags);
    auto mod = std::make_unique<Module>();
    auto fn = new Function(); fn->id = "fn1"; fn->name = "f";

    // Valid Subject 5
    auto cc = new CallingConvAnnotation(); cc->convention = "cdecl";
    fn->addChild("annotations", cc);
    auto sh = new ShimAnnotation(); sh->strategy = "vtable";
    fn->addChild("annotations", sh);

    // Valid Subject 6 (no Inline+TailCall conflict)
    auto inl = new InlineAnnotation(); inl->mode = "Hint";
    fn->addChild("annotations", inl);
    auto lo = new LoopAnnotation(); lo->hint = "vectorize"; lo->factor = 4;
    fn->addChild("annotations", lo);
    auto al = new AlignAnnotation(); al->bytes = 16;
    fn->addChild("annotations", al);
    auto ov = new OverflowAnnotation(); ov->behavior = "wrap";
    fn->addChild("annotations", ov);

    // Valid Subject 7
    auto me = new MetaAnnotation(); me->state = "quoted"; me->phase = "compile";
    fn->addChild("annotations", me);
    auto te = new TemplateAnnotation(); te->specialization = "monomorphize";
    fn->addChild("annotations", te);

    // Valid Subject 8
    auto po = new PolicyAnnotation(); po->strictness = "high"; po->perf = "critical"; po->style = "idiomatic";
    fn->addChild("annotations", po);

    mod->addChild("functions", fn);

    AnnotationValidatorExtended v;
    auto diags = v.validate(mod.get());
    CHECK(diags.empty(), "expected 0 diagnostics for all valid annotations, got " + std::to_string(diags.size()));
    PASS();
}

int main() {
    std::cout << "=== Step 298: Subject 5-8 Validation Tests ===\n";
    test_callingconv_invalid();
    test_shim_invalid();
    test_inline_always_tailcall();
    test_inline_hint_tailcall();
    test_loop_invalid_hint();
    test_loop_unroll_factor_zero();
    test_align_invalid();
    test_overflow_invalid();
    test_meta_invalid_state();
    test_template_invalid();
    test_policy_invalid_strictness();
    test_all_valid_subject_5_8();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
