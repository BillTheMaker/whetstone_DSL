// Step 278: Shim & Escape Hatch Annotations (12 tests)
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

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

static void test_intrinsic() {
    IntrinsicAnnotation a;
    check(a.conceptType == "IntrinsicAnnotation", "IntrinsicAnnotation conceptType");
    a.instruction = "_mm256_add_ps"; a.arch = "x86_avx2";
    check(a.instruction == "_mm256_add_ps" && a.arch == "x86_avx2", "fields set");
}

static void test_raw() {
    RawAnnotation a; a.language = "c"; a.code = "asm volatile(\"nop\")";
    check(a.conceptType == "RawAnnotation", "RawAnnotation conceptType");
    check(a.language == "c" && a.code == "asm volatile(\"nop\")", "fields set");
}

static void test_callingconv_link_shim() {
    CallingConvAnnotation cc; cc.convention = "stdcall";
    check(cc.convention == "stdcall", "CallingConvAnnotation field");
    LinkAnnotation la; la.symbolName = "printf"; la.library = "libc.so";
    check(la.symbolName == "printf" && la.library == "libc.so", "LinkAnnotation fields");
    ShimAnnotation sa; sa.strategy = "vtable";
    check(sa.strategy == "vtable", "ShimAnnotation field");
}

static void test_marker_and_opaque() {
    PointerArithmeticAnnotation pa;
    check(pa.conceptType == "PointerArithmeticAnnotation", "PointerArithmeticAnnotation conceptType");
    OpaqueAnnotation oa; oa.reason = "platform-specific ABI";
    check(oa.reason == "platform-specific ABI", "OpaqueAnnotation field");
}

static void test_intrinsic_roundtrip() {
    auto* a = new IntrinsicAnnotation(); a->id = "i1";
    a->instruction = "_mm_mul_ps"; a->arch = "x86_sse";
    json j = toJson(a);
    check(j["properties"]["instruction"] == "_mm_mul_ps", "JSON instruction");
    ASTNode* r = fromJson(j);
    auto* ri = dynamic_cast<IntrinsicAnnotation*>(r);
    check(ri && ri->instruction == "_mm_mul_ps" && ri->arch == "x86_sse", "roundtrip");
    delete a; delete r;
}

static void test_link_roundtrip() {
    auto* a = new LinkAnnotation(); a->id = "l1";
    a->symbolName = "malloc"; a->library = "libc";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rl = dynamic_cast<LinkAnnotation*>(r);
    check(rl && rl->symbolName == "malloc" && rl->library == "libc", "LinkAnnotation roundtrip");
    delete a; delete r;
}

static void test_raw_roundtrip() {
    auto* a = new RawAnnotation(); a->id = "r1";
    a->language = "cpp"; a->code = "__attribute__((packed))";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rr = dynamic_cast<RawAnnotation*>(r);
    check(rr && rr->language == "cpp" && rr->code == "__attribute__((packed))", "RawAnnotation roundtrip");
    delete a; delete r;
}

static void test_create_node_factory() {
    std::vector<std::string> types = {
        "IntrinsicAnnotation", "RawAnnotation", "CallingConvAnnotation",
        "LinkAnnotation", "ShimAnnotation", "PointerArithmeticAnnotation",
        "OpaqueAnnotation"
    };
    for (const auto& t : types) {
        ASTNode* n = createNode(t);
        check(n && n->conceptType == t, "createNode(" + t + ")");
        delete n;
    }
}

static void test_compact_ast_intrinsic() {
    auto* f = new Function(); f->id = "f1"; f->name = "simd_add";
    auto* a = new IntrinsicAnnotation(); a->instruction = "_mm_add"; a->arch = "sse";
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("intrinsic") && sem["intrinsic"]["instruction"] == "_mm_add", "intrinsic in compact AST");
    delete f;
}

static void test_compact_ast_shim_pointer() {
    auto* f = new Function(); f->id = "f2"; f->name = "ffi_bridge";
    auto* s = new ShimAnnotation(); s->strategy = "trampoline";
    auto* p = new PointerArithmeticAnnotation();
    f->addChild("annotations", s); f->addChild("annotations", p);
    json sem = extractSemanticSummary(f);
    check(sem["shim"] == "trampoline", "shim in compact AST");
    check(sem["pointerArithmetic"] == true, "pointerArithmetic in compact AST");
    delete f;
}

static void test_all_shim_annotations() {
    auto* f = new Function(); f->id = "f3"; f->name = "fullShim";
    f->addChild("annotations", [&]{ auto* a = new IntrinsicAnnotation(); a->instruction="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new RawAnnotation(); a->code="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new CallingConvAnnotation(); a->convention="cdecl"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new LinkAnnotation(); a->symbolName="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new ShimAnnotation(); a->strategy="cast"; return a; }());
    f->addChild("annotations", new PointerArithmeticAnnotation());
    f->addChild("annotations", [&]{ auto* a = new OpaqueAnnotation(); a->reason="x"; return a; }());
    json sem = extractSemanticSummary(f);
    check(sem.contains("intrinsic") && sem.contains("raw") &&
          sem.contains("callingConv") && sem.contains("link") &&
          sem.contains("shim") && sem.contains("pointerArithmetic") &&
          sem.contains("opaque"), "all 7 shim annotations in summary");
    delete f;
}

static void test_callingconv_roundtrip() {
    auto* a = new CallingConvAnnotation(); a->id="cc1"; a->convention="fastcall";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rc = dynamic_cast<CallingConvAnnotation*>(r);
    check(rc && rc->convention == "fastcall", "CallingConvAnnotation roundtrip");
    delete a; delete r;
}

int main() {
    std::cout << "=== Step 278: Shim & Escape Hatch Annotations ===\n";
    test_intrinsic(); test_raw(); test_callingconv_link_shim();
    test_marker_and_opaque(); test_intrinsic_roundtrip();
    test_link_roundtrip(); test_raw_roundtrip();
    test_create_node_factory(); test_compact_ast_intrinsic();
    test_compact_ast_shim_pointer(); test_all_shim_annotations();
    test_callingconv_roundtrip();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
