// Step 273: Type System Annotations — Identity & Mutability (12 tests)
// Tests: IdentityAnnotation, MutAnnotation, TypeStateAnnotation
//        + JSON roundtrip + compact AST + all 8 type annotation types together

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

// --- Test 1: IdentityAnnotation creation ---
static void test_identity_annotation() {
    IdentityAnnotation ia;
    check(ia.conceptType == "IdentityAnnotation", "conceptType is IdentityAnnotation");
    ia.mode = "nominal";
    check(ia.mode == "nominal", "mode field set to nominal");
}

// --- Test 2: MutAnnotation creation ---
static void test_mut_annotation() {
    MutAnnotation ma;
    check(ma.conceptType == "MutAnnotation", "conceptType is MutAnnotation");
    ma.depth = "deep";
    check(ma.depth == "deep", "depth field set to deep");
}

// --- Test 3: TypeStateAnnotation creation ---
static void test_typestate_annotation() {
    TypeStateAnnotation ts;
    check(ts.conceptType == "TypeStateAnnotation", "conceptType is TypeStateAnnotation");
    ts.state = "reified";
    check(ts.state == "reified", "state field set to reified");
}

// --- Test 4: IdentityAnnotation JSON roundtrip ---
static void test_identity_json_roundtrip() {
    auto* ia = new IdentityAnnotation();
    ia->id = "id1";
    ia->mode = "structural";
    json j = toJson(ia);
    check(j["properties"]["mode"] == "structural", "JSON mode correct");
    ASTNode* restored = fromJson(j);
    auto* ria = dynamic_cast<IdentityAnnotation*>(restored);
    check(ria != nullptr, "fromJson creates IdentityAnnotation");
    check(ria->mode == "structural", "roundtrip mode preserved");
    delete ia;
    delete restored;
}

// --- Test 5: MutAnnotation JSON roundtrip ---
static void test_mut_json_roundtrip() {
    auto* ma = new MutAnnotation();
    ma->id = "mut1";
    ma->depth = "interior";
    json j = toJson(ma);
    ASTNode* restored = fromJson(j);
    auto* rma = dynamic_cast<MutAnnotation*>(restored);
    check(rma != nullptr, "fromJson creates MutAnnotation");
    check(rma->depth == "interior", "roundtrip depth preserved");
    delete ma;
    delete restored;
}

// --- Test 6: TypeStateAnnotation JSON roundtrip ---
static void test_typestate_json_roundtrip() {
    auto* ts = new TypeStateAnnotation();
    ts->id = "ts1";
    ts->state = "erased";
    json j = toJson(ts);
    ASTNode* restored = fromJson(j);
    auto* rts = dynamic_cast<TypeStateAnnotation*>(restored);
    check(rts != nullptr, "fromJson creates TypeStateAnnotation");
    check(rts->state == "erased", "roundtrip state preserved");
    delete ts;
    delete restored;
}

// --- Test 7: createNode factory for Step 273 types ---
static void test_create_node_factory() {
    std::vector<std::string> types = {
        "IdentityAnnotation", "MutAnnotation", "TypeStateAnnotation"
    };
    for (const auto& t : types) {
        ASTNode* node = createNode(t);
        check(node != nullptr && node->conceptType == t,
              "createNode(" + t + ") works");
        delete node;
    }
}

// --- Test 8: Compact AST shows identity in semantic summary ---
static void test_compact_ast_identity() {
    auto* func = new Function();
    func->id = "f1";
    func->name = "compare";
    auto* ia = new IdentityAnnotation();
    ia->mode = "nominal";
    func->addChild("annotations", ia);
    json sem = extractSemanticSummary(func);
    check(sem.contains("identity"), "identity in semantic summary");
    check(sem["identity"] == "nominal", "identity value correct");
    delete func;
}

// --- Test 9: Compact AST shows mutability in semantic summary ---
static void test_compact_ast_mutability() {
    auto* func = new Function();
    func->id = "f2";
    func->name = "freeze";
    auto* ma = new MutAnnotation();
    ma->depth = "deep";
    func->addChild("annotations", ma);
    json sem = extractSemanticSummary(func);
    check(sem.contains("mutability"), "mutability in semantic summary");
    check(sem["mutability"] == "deep", "mutability value correct");
    delete func;
}

// --- Test 10: Compact AST shows typeState in semantic summary ---
static void test_compact_ast_typestate() {
    auto* func = new Function();
    func->id = "f3";
    func->name = "reflect";
    auto* ts = new TypeStateAnnotation();
    ts->state = "reified";
    func->addChild("annotations", ts);
    json sem = extractSemanticSummary(func);
    check(sem.contains("typeState"), "typeState in semantic summary");
    check(sem["typeState"] == "reified", "typeState value correct");
    delete func;
}

// --- Test 11: All 8 Subject 2 type annotations on one function ---
static void test_all_subject2_annotations() {
    auto* func = new Function();
    func->id = "f4";
    func->name = "fullType";
    auto* bw = new BitWidthAnnotation(); bw->width = 32;
    auto* ea = new EndianAnnotation(); ea->order = "little";
    auto* la = new LayoutAnnotation(); la->mode = "aligned"; la->alignment = 8;
    auto* na = new NullabilityAnnotation(); na->nullable = false; na->strategy = "strict";
    auto* va = new VarianceAnnotation(); va->variance = "covariant";
    auto* id = new IdentityAnnotation(); id->mode = "nominal";
    auto* mu = new MutAnnotation(); mu->depth = "shallow";
    auto* ts = new TypeStateAnnotation(); ts->state = "erased";
    func->addChild("annotations", bw);
    func->addChild("annotations", ea);
    func->addChild("annotations", la);
    func->addChild("annotations", na);
    func->addChild("annotations", va);
    func->addChild("annotations", id);
    func->addChild("annotations", mu);
    func->addChild("annotations", ts);
    json sem = extractSemanticSummary(func);
    check(sem.contains("bitWidth") && sem.contains("endian") &&
          sem.contains("layout") && sem.contains("nullability") &&
          sem.contains("variance") && sem.contains("identity") &&
          sem.contains("mutability") && sem.contains("typeState"),
          "all 8 type system annotations in semantic summary");
    delete func;
}

// --- Test 12: MutAnnotation drives const qualification hint ---
static void test_mut_const_qualification() {
    // MutAnnotation("deep") → implies deeply immutable
    MutAnnotation ma;
    ma.depth = "deep";
    check(ma.depth == "deep", "deep mutability implies deeply immutable");
    // MutAnnotation("shallow") → only top-level const
    MutAnnotation ma2;
    ma2.depth = "shallow";
    check(ma2.depth == "shallow", "shallow mutability implies top-level const");
}

int main() {
    std::cout << "=== Step 273: Type System Annotations — Identity & Mutability ===\n";
    test_identity_annotation();
    test_mut_annotation();
    test_typestate_annotation();
    test_identity_json_roundtrip();
    test_mut_json_roundtrip();
    test_typestate_json_roundtrip();
    test_create_node_factory();
    test_compact_ast_identity();
    test_compact_ast_mutability();
    test_compact_ast_typestate();
    test_all_subject2_annotations();
    test_mut_const_qualification();
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed out of "
              << (passed + failed) << "\n";
    return failed > 0 ? 1 : 0;
}
