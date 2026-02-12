// Step 276: Scope & Namespace Annotations (12 tests)
// Tests: BindingAnnotation, LookupAnnotation, CaptureAnnotation,
//        VisibilityAnnotation, NamespaceAnnotation, ScopeAnnotation
//        + JSON roundtrip + compact AST

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

static int passed = 0;
static int failed = 0;

static void check(bool cond, const std::string& name) {
    if (cond) { std::cout << "  PASS: " << name << "\n"; ++passed; }
    else      { std::cout << "  FAIL: " << name << "\n"; ++failed; }
}

// --- Test 1: BindingAnnotation creation ---
static void test_binding_annotation() {
    BindingAnnotation ba;
    check(ba.conceptType == "BindingAnnotation", "conceptType is BindingAnnotation");
    ba.time = "static";
    check(ba.time == "static", "time field set to static");
}

// --- Test 2: LookupAnnotation creation ---
static void test_lookup_annotation() {
    LookupAnnotation la;
    check(la.conceptType == "LookupAnnotation", "conceptType is LookupAnnotation");
    la.mode = "lexical";
    check(la.mode == "lexical", "mode field set to lexical");
}

// --- Test 3: CaptureAnnotation creation ---
static void test_capture_annotation() {
    CaptureAnnotation ca;
    check(ca.conceptType == "CaptureAnnotation", "conceptType is CaptureAnnotation");
    ca.strategy = "move";
    check(ca.strategy == "move", "strategy field set to move");
}

// --- Test 4: VisibilityAnnotation, NamespaceAnnotation, ScopeAnnotation ---
static void test_visibility_namespace_scope() {
    VisibilityAnnotation va;
    check(va.conceptType == "VisibilityAnnotation", "VisibilityAnnotation conceptType");
    va.level = "friend";
    check(va.level == "friend", "level set to friend");

    NamespaceAnnotation na;
    check(na.conceptType == "NamespaceAnnotation", "NamespaceAnnotation conceptType");
    na.style = "qualified";
    check(na.style == "qualified", "style set to qualified");

    ScopeAnnotation sa;
    check(sa.conceptType == "ScopeAnnotation", "ScopeAnnotation conceptType");
    sa.kind = "singleton";
    check(sa.kind == "singleton", "kind set to singleton");
}

// --- Test 5: CaptureAnnotation JSON roundtrip ---
static void test_capture_json_roundtrip() {
    auto* ca = new CaptureAnnotation();
    ca->id = "c1";
    ca->strategy = "ref";
    json j = toJson(ca);
    check(j["properties"]["strategy"] == "ref", "JSON strategy correct");
    ASTNode* restored = fromJson(j);
    auto* rca = dynamic_cast<CaptureAnnotation*>(restored);
    check(rca != nullptr, "fromJson creates CaptureAnnotation");
    check(rca->strategy == "ref", "roundtrip strategy preserved");
    delete ca;
    delete restored;
}

// --- Test 6: VisibilityAnnotation JSON roundtrip ---
static void test_visibility_json_roundtrip() {
    auto* va = new VisibilityAnnotation();
    va->id = "v1";
    va->level = "internal";
    json j = toJson(va);
    ASTNode* restored = fromJson(j);
    auto* rva = dynamic_cast<VisibilityAnnotation*>(restored);
    check(rva != nullptr, "fromJson creates VisibilityAnnotation");
    check(rva->level == "internal", "roundtrip level preserved");
    delete va;
    delete restored;
}

// --- Test 7: ScopeAnnotation JSON roundtrip ---
static void test_scope_json_roundtrip() {
    auto* sa = new ScopeAnnotation();
    sa->id = "s1";
    sa->kind = "global_leaked";
    json j = toJson(sa);
    ASTNode* restored = fromJson(j);
    auto* rsa = dynamic_cast<ScopeAnnotation*>(restored);
    check(rsa != nullptr, "fromJson creates ScopeAnnotation");
    check(rsa->kind == "global_leaked", "roundtrip kind preserved");
    delete sa;
    delete restored;
}

// --- Test 8: createNode factory for Step 276 types ---
static void test_create_node_factory() {
    std::vector<std::string> types = {
        "BindingAnnotation", "LookupAnnotation", "CaptureAnnotation",
        "VisibilityAnnotation", "NamespaceAnnotation", "ScopeAnnotation"
    };
    for (const auto& t : types) {
        ASTNode* node = createNode(t);
        check(node != nullptr && node->conceptType == t,
              "createNode(" + t + ") works");
        delete node;
    }
}

// --- Test 9: Compact AST shows binding and lookup ---
static void test_compact_ast_binding_lookup() {
    auto* func = new Function();
    func->id = "f1";
    func->name = "resolve";
    auto* ba = new BindingAnnotation(); ba->time = "dynamic";
    auto* la = new LookupAnnotation(); la->mode = "hoisted";
    func->addChild("annotations", ba);
    func->addChild("annotations", la);
    json sem = extractSemanticSummary(func);
    check(sem["binding"] == "dynamic", "binding in semantic summary");
    check(sem["lookup"] == "hoisted", "lookup in semantic summary");
    delete func;
}

// --- Test 10: Compact AST shows capture and visibility ---
static void test_compact_ast_capture_visibility() {
    auto* func = new Function();
    func->id = "f2";
    func->name = "closureFactory";
    auto* ca = new CaptureAnnotation(); ca->strategy = "value";
    auto* va = new VisibilityAnnotation(); va->level = "private";
    func->addChild("annotations", ca);
    func->addChild("annotations", va);
    json sem = extractSemanticSummary(func);
    check(sem["capture"] == "value", "capture in semantic summary");
    check(sem["visibility"] == "private", "visibility in semantic summary");
    delete func;
}

// --- Test 11: All 6 scope annotations on one function ---
static void test_all_scope_annotations() {
    auto* func = new Function();
    func->id = "f3";
    func->name = "fullScope";
    func->addChild("annotations", [&]{ auto* a = new BindingAnnotation(); a->time = "static"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new LookupAnnotation(); a->mode = "lexical"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new CaptureAnnotation(); a->strategy = "move"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new VisibilityAnnotation(); a->level = "public"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new NamespaceAnnotation(); a->style = "flat"; return a; }());
    func->addChild("annotations", [&]{ auto* a = new ScopeAnnotation(); a->kind = "local"; return a; }());
    json sem = extractSemanticSummary(func);
    check(sem.contains("binding") && sem.contains("lookup") &&
          sem.contains("capture") && sem.contains("visibility") &&
          sem.contains("namespace") && sem.contains("scope"),
          "all 6 scope annotations in semantic summary");
    delete func;
}

// --- Test 12: BindingAnnotation and NamespaceAnnotation JSON roundtrip ---
static void test_binding_namespace_roundtrip() {
    auto* ba = new BindingAnnotation();
    ba->id = "b1";
    ba->time = "dynamic";
    json jb = toJson(ba);
    ASTNode* rb = fromJson(jb);
    auto* rba = dynamic_cast<BindingAnnotation*>(rb);
    check(rba != nullptr && rba->time == "dynamic", "BindingAnnotation roundtrip");

    auto* na = new NamespaceAnnotation();
    na->id = "n1";
    na->style = "qualified";
    json jn = toJson(na);
    ASTNode* rn = fromJson(jn);
    auto* rna = dynamic_cast<NamespaceAnnotation*>(rn);
    check(rna != nullptr && rna->style == "qualified", "NamespaceAnnotation roundtrip");

    delete ba; delete rb; delete na; delete rn;
}

int main() {
    std::cout << "=== Step 276: Scope & Namespace Annotations ===\n";
    test_binding_annotation();
    test_lookup_annotation();
    test_capture_annotation();
    test_visibility_namespace_scope();
    test_capture_json_roundtrip();
    test_visibility_json_roundtrip();
    test_scope_json_roundtrip();
    test_create_node_factory();
    test_compact_ast_binding_lookup();
    test_compact_ast_capture_visibility();
    test_all_scope_annotations();
    test_binding_namespace_roundtrip();
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed out of "
              << (passed + failed) << "\n";
    return failed > 0 ? 1 : 0;
}
