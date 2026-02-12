// Step 281: Meta-Programming Annotations (12 tests)
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

static void test_meta() {
    MetaAnnotation a; a.state = "quoted"; a.phase = "compile";
    check(a.conceptType == "MetaAnnotation", "conceptType");
    check(a.state == "quoted" && a.phase == "compile", "fields");
}

static void test_symbol() {
    SymbolAnnotation a; a.mode = "gensym";
    check(a.conceptType == "SymbolAnnotation", "conceptType");
    check(a.mode == "gensym", "mode field");
}

static void test_evaluate() {
    EvaluateAnnotation a; a.phase = "compile_time";
    check(a.conceptType == "EvaluateAnnotation", "conceptType");
    check(a.phase == "compile_time", "phase field");
}

static void test_template_synthetic() {
    TemplateAnnotation t; t.specialization = "monomorphize";
    check(t.specialization == "monomorphize", "TemplateAnnotation field");
    SyntheticAnnotation s; s.generator = "macro_expand"; s.isStructuralRisk = true;
    check(s.generator == "macro_expand" && s.isStructuralRisk, "SyntheticAnnotation fields");
}

static void test_meta_roundtrip() {
    auto* a = new MetaAnnotation(); a->id = "m1"; a->state = "unquoted"; a->phase = "runtime";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rm = dynamic_cast<MetaAnnotation*>(r);
    check(rm && rm->state == "unquoted" && rm->phase == "runtime", "MetaAnnotation roundtrip");
    delete a; delete r;
}

static void test_template_roundtrip() {
    auto* a = new TemplateAnnotation(); a->id = "t1"; a->specialization = "erasure";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rt = dynamic_cast<TemplateAnnotation*>(r);
    check(rt && rt->specialization == "erasure", "TemplateAnnotation roundtrip");
    delete a; delete r;
}

static void test_synthetic_roundtrip() {
    auto* a = new SyntheticAnnotation(); a->id = "s1";
    a->generator = "derive_macro"; a->isStructuralRisk = true;
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rs = dynamic_cast<SyntheticAnnotation*>(r);
    check(rs && rs->generator == "derive_macro" && rs->isStructuralRisk, "SyntheticAnnotation roundtrip");
    delete a; delete r;
}

static void test_create_node_factory() {
    for (const auto& t : {"MetaAnnotation", "SymbolAnnotation", "EvaluateAnnotation",
                           "TemplateAnnotation", "SyntheticAnnotation"}) {
        ASTNode* n = createNode(t);
        check(n && n->conceptType == t, std::string("createNode(") + t + ")");
        delete n;
    }
}

static void test_compact_ast_meta() {
    auto* f = new Function(); f->id = "f1"; f->name = "macroExpand";
    auto* a = new MetaAnnotation(); a->state = "quoted"; a->phase = "compile";
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("meta") && sem["meta"]["state"] == "quoted", "meta in compact AST");
    delete f;
}

static void test_compact_ast_template() {
    auto* f = new Function(); f->id = "f2"; f->name = "generic";
    auto* a = new TemplateAnnotation(); a->specialization = "trait";
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem["template"] == "trait", "template in compact AST");
    delete f;
}

static void test_compact_ast_synthetic() {
    auto* f = new Function(); f->id = "f3"; f->name = "generated";
    auto* a = new SyntheticAnnotation(); a->generator = "proc_macro"; a->isStructuralRisk = true;
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("synthetic") && sem["synthetic"]["generator"] == "proc_macro" &&
          sem["synthetic"]["risk"] == true, "synthetic in compact AST");
    delete f;
}

static void test_all_meta_annotations() {
    auto* f = new Function(); f->id = "f4"; f->name = "full";
    f->addChild("annotations", [&]{ auto* a = new MetaAnnotation(); a->state="quoted"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new SymbolAnnotation(); a->mode="interned"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new EvaluateAnnotation(); a->phase="runtime"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new TemplateAnnotation(); a->specialization="trait"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new SyntheticAnnotation(); a->generator="x"; return a; }());
    json sem = extractSemanticSummary(f);
    check(sem.contains("meta") && sem.contains("symbol") &&
          sem.contains("evaluate") && sem.contains("template") &&
          sem.contains("synthetic"), "all 5 meta annotations in summary");
    delete f;
}

int main() {
    std::cout << "=== Step 281: Meta-Programming Annotations ===\n";
    test_meta(); test_symbol(); test_evaluate();
    test_template_synthetic(); test_meta_roundtrip();
    test_template_roundtrip(); test_synthetic_roundtrip();
    test_create_node_factory(); test_compact_ast_meta();
    test_compact_ast_template(); test_compact_ast_synthetic();
    test_all_meta_annotations();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
