// Step 280: Optimization Annotation Completion (12 tests)
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

static void test_tailcall() {
    TailCallAnnotation a;
    check(a.conceptType == "TailCallAnnotation", "TailCallAnnotation conceptType");
}

static void test_loop() {
    LoopAnnotation a; a.hint = "unroll"; a.factor = 4;
    check(a.conceptType == "LoopAnnotation", "LoopAnnotation conceptType");
    check(a.hint == "unroll" && a.factor == 4, "fields");
}

static void test_data_align() {
    DataAnnotation d; d.hint = "prefetch";
    check(d.hint == "prefetch", "DataAnnotation hint");
    AlignAnnotation a; a.bytes = 64;
    check(a.bytes == 64, "AlignAnnotation bytes");
}

static void test_pack_bounds_overflow() {
    PackAnnotation p;
    check(p.conceptType == "PackAnnotation", "PackAnnotation conceptType");
    BoundsCheckAnnotation b; b.enabled = false;
    check(!b.enabled, "BoundsCheckAnnotation enabled=false");
    OverflowAnnotation o; o.behavior = "wrap";
    check(o.behavior == "wrap", "OverflowAnnotation behavior");
}

static void test_loop_roundtrip() {
    auto* a = new LoopAnnotation(); a->id = "l1"; a->hint = "vectorize"; a->factor = 8;
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rl = dynamic_cast<LoopAnnotation*>(r);
    check(rl && rl->hint == "vectorize" && rl->factor == 8, "LoopAnnotation roundtrip");
    delete a; delete r;
}

static void test_align_roundtrip() {
    auto* a = new AlignAnnotation(); a->id = "a1"; a->bytes = 32;
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* ra = dynamic_cast<AlignAnnotation*>(r);
    check(ra && ra->bytes == 32, "AlignAnnotation roundtrip");
    delete a; delete r;
}

static void test_overflow_roundtrip() {
    auto* a = new OverflowAnnotation(); a->id = "o1"; a->behavior = "saturation";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* ro = dynamic_cast<OverflowAnnotation*>(r);
    check(ro && ro->behavior == "saturation", "OverflowAnnotation roundtrip");
    delete a; delete r;
}

static void test_create_node_factory() {
    for (const auto& t : {"TailCallAnnotation", "LoopAnnotation", "DataAnnotation",
                           "AlignAnnotation", "PackAnnotation",
                           "BoundsCheckAnnotation", "OverflowAnnotation"}) {
        ASTNode* n = createNode(t);
        check(n && n->conceptType == t, std::string("createNode(") + t + ")");
        delete n;
    }
}

static void test_compact_ast_loop() {
    auto* f = new Function(); f->id = "f1"; f->name = "vectorOp";
    auto* a = new LoopAnnotation(); a->hint = "vectorize"; a->factor = 4;
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("loop") && sem["loop"]["hint"] == "vectorize", "loop in compact AST");
    delete f;
}

static void test_compact_ast_markers() {
    auto* f = new Function(); f->id = "f2"; f->name = "recursive";
    f->addChild("annotations", new TailCallAnnotation());
    f->addChild("annotations", new PackAnnotation());
    json sem = extractSemanticSummary(f);
    check(sem["tailCall"] == true && sem["pack"] == true, "markers in compact AST");
    delete f;
}

static void test_compact_ast_safety() {
    auto* f = new Function(); f->id = "f3"; f->name = "unsafeOp";
    auto* b = new BoundsCheckAnnotation(); b->enabled = false;
    auto* o = new OverflowAnnotation(); o->behavior = "panic";
    f->addChild("annotations", b); f->addChild("annotations", o);
    json sem = extractSemanticSummary(f);
    check(sem["boundsCheck"] == false && sem["overflow"] == "panic", "safety annotations");
    delete f;
}

static void test_all_optimization_annotations() {
    auto* f = new Function(); f->id = "f4"; f->name = "full";
    f->addChild("annotations", new TailCallAnnotation());
    f->addChild("annotations", [&]{ auto* a = new LoopAnnotation(); a->hint="fuse"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new DataAnnotation(); a->hint="restrict"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new AlignAnnotation(); a->bytes=16; return a; }());
    f->addChild("annotations", new PackAnnotation());
    f->addChild("annotations", [&]{ auto* a = new BoundsCheckAnnotation(); a->enabled=true; return a; }());
    f->addChild("annotations", [&]{ auto* a = new OverflowAnnotation(); a->behavior="wrap"; return a; }());
    json sem = extractSemanticSummary(f);
    check(sem.contains("tailCall") && sem.contains("loop") &&
          sem.contains("data") && sem.contains("align") &&
          sem.contains("pack") && sem.contains("boundsCheck") &&
          sem.contains("overflow"), "all 7 optimization annotations in summary");
    delete f;
}

int main() {
    std::cout << "=== Step 280: Optimization Annotation Completion ===\n";
    test_tailcall(); test_loop(); test_data_align();
    test_pack_bounds_overflow(); test_loop_roundtrip();
    test_align_roundtrip(); test_overflow_roundtrip();
    test_create_node_factory(); test_compact_ast_loop();
    test_compact_ast_markers(); test_compact_ast_safety();
    test_all_optimization_annotations();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
