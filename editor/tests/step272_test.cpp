// Step 272: Type System Annotations — Layout & Constraints (12 tests)
// Tests: BitWidthAnnotation, EndianAnnotation, LayoutAnnotation,
//        NullabilityAnnotation, VarianceAnnotation
//        + JSON roundtrip + compact AST integration

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

// --- Test 1: BitWidthAnnotation creation and fields ---
static void test_bitwidth_annotation() {
    BitWidthAnnotation bw;
    check(bw.conceptType == "BitWidthAnnotation", "conceptType is BitWidthAnnotation");
    bw.width = 32;
    check(bw.width == 32, "width field set to 32");
}

// --- Test 2: EndianAnnotation creation and fields ---
static void test_endian_annotation() {
    EndianAnnotation ea;
    check(ea.conceptType == "EndianAnnotation", "conceptType is EndianAnnotation");
    ea.order = "big";
    check(ea.order == "big", "order field set to big");
}

// --- Test 3: LayoutAnnotation creation and fields ---
static void test_layout_annotation() {
    LayoutAnnotation la;
    check(la.conceptType == "LayoutAnnotation", "conceptType is LayoutAnnotation");
    la.mode = "packed";
    la.alignment = 8;
    check(la.mode == "packed", "mode field set to packed");
    check(la.alignment == 8, "alignment field set to 8");
}

// --- Test 4: NullabilityAnnotation creation and fields ---
static void test_nullability_annotation() {
    NullabilityAnnotation na;
    check(na.conceptType == "NullabilityAnnotation", "conceptType is NullabilityAnnotation");
    na.nullable = true;
    na.strategy = "strict";
    check(na.nullable == true, "nullable field set to true");
    check(na.strategy == "strict", "strategy field set to strict");
}

// --- Test 5: VarianceAnnotation creation and fields ---
static void test_variance_annotation() {
    VarianceAnnotation va;
    check(va.conceptType == "VarianceAnnotation", "conceptType is VarianceAnnotation");
    va.variance = "covariant";
    check(va.variance == "covariant", "variance field set to covariant");
}

// --- Test 6: BitWidthAnnotation JSON roundtrip ---
static void test_bitwidth_json_roundtrip() {
    auto* bw = new BitWidthAnnotation();
    bw->id = "bw1";
    bw->width = 64;
    json j = toJson(bw);
    check(j["concept"] == "BitWidthAnnotation", "JSON concept correct");
    check(j["properties"]["width"] == 64, "JSON width correct");
    ASTNode* restored = fromJson(j);
    auto* rbw = dynamic_cast<BitWidthAnnotation*>(restored);
    check(rbw != nullptr, "fromJson creates BitWidthAnnotation");
    check(rbw->width == 64, "roundtrip width preserved");
    delete bw;
    delete restored;
}

// --- Test 7: LayoutAnnotation JSON roundtrip ---
static void test_layout_json_roundtrip() {
    auto* la = new LayoutAnnotation();
    la->id = "la1";
    la->mode = "aligned";
    la->alignment = 16;
    json j = toJson(la);
    ASTNode* restored = fromJson(j);
    auto* rla = dynamic_cast<LayoutAnnotation*>(restored);
    check(rla != nullptr, "fromJson creates LayoutAnnotation");
    check(rla->mode == "aligned", "roundtrip mode preserved");
    check(rla->alignment == 16, "roundtrip alignment preserved");
    delete la;
    delete restored;
}

// --- Test 8: NullabilityAnnotation JSON roundtrip ---
static void test_nullability_json_roundtrip() {
    auto* na = new NullabilityAnnotation();
    na->id = "na1";
    na->nullable = true;
    na->strategy = "nullable";
    json j = toJson(na);
    check(j["properties"]["nullable"] == true, "JSON nullable correct");
    ASTNode* restored = fromJson(j);
    auto* rna = dynamic_cast<NullabilityAnnotation*>(restored);
    check(rna != nullptr, "fromJson creates NullabilityAnnotation");
    check(rna->nullable == true, "roundtrip nullable preserved");
    check(rna->strategy == "nullable", "roundtrip strategy preserved");
    delete na;
    delete restored;
}

// --- Test 9: All 5 types survive createNode factory ---
static void test_create_node_factory() {
    std::vector<std::string> types = {
        "BitWidthAnnotation", "EndianAnnotation", "LayoutAnnotation",
        "NullabilityAnnotation", "VarianceAnnotation"
    };
    for (const auto& t : types) {
        ASTNode* node = createNode(t);
        check(node != nullptr && node->conceptType == t,
              "createNode(" + t + ") works");
        delete node;
    }
}

// --- Test 10: Compact AST shows bitWidth in semantic summary ---
static void test_compact_ast_bitwidth() {
    auto* func = new Function();
    func->id = "f1";
    func->name = "getInt";
    auto* bw = new BitWidthAnnotation();
    bw->width = 32;
    func->addChild("annotations", bw);
    json sem = extractSemanticSummary(func);
    check(!sem.empty(), "semantic summary not empty");
    check(sem.contains("bitWidth"), "bitWidth in semantic summary");
    check(sem["bitWidth"] == 32, "bitWidth value correct");
    delete func;
}

// --- Test 11: Compact AST shows layout in semantic summary ---
static void test_compact_ast_layout() {
    auto* func = new Function();
    func->id = "f2";
    func->name = "pack";
    auto* la = new LayoutAnnotation();
    la->mode = "packed";
    la->alignment = 4;
    func->addChild("annotations", la);
    json sem = extractSemanticSummary(func);
    check(sem.contains("layout"), "layout in semantic summary");
    check(sem["layout"]["mode"] == "packed", "layout mode correct");
    check(sem["layout"]["alignment"] == 4, "layout alignment correct");
    delete func;
}

// --- Test 12: Multiple type annotations on one node ---
static void test_multiple_type_annotations() {
    auto* func = new Function();
    func->id = "f3";
    func->name = "convert";
    auto* bw = new BitWidthAnnotation();
    bw->width = 16;
    func->addChild("annotations", bw);
    auto* ea = new EndianAnnotation();
    ea->order = "little";
    func->addChild("annotations", ea);
    auto* va = new VarianceAnnotation();
    va->variance = "invariant";
    func->addChild("annotations", va);
    json sem = extractSemanticSummary(func);
    check(sem["bitWidth"] == 16, "bitWidth present in multi-annotation");
    check(sem["endian"] == "little", "endian present in multi-annotation");
    check(sem["variance"] == "invariant", "variance present in multi-annotation");
    delete func;
}

int main() {
    std::cout << "=== Step 272: Type System Annotations — Layout & Constraints ===\n";
    test_bitwidth_annotation();
    test_endian_annotation();
    test_layout_annotation();
    test_nullability_annotation();
    test_variance_annotation();
    test_bitwidth_json_roundtrip();
    test_layout_json_roundtrip();
    test_nullability_json_roundtrip();
    test_create_node_factory();
    test_compact_ast_bitwidth();
    test_compact_ast_layout();
    test_multiple_type_annotations();
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed out of "
              << (passed + failed) << "\n";
    return failed > 0 ? 1 : 0;
}
