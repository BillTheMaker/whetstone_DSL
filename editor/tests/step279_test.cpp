// Step 279: Platform & Provenance Annotations (12 tests)
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

static void test_target() {
    TargetAnnotation a; a.platform = "linux"; a.arch = "x86_64";
    check(a.conceptType == "TargetAnnotation", "conceptType");
    check(a.platform == "linux" && a.arch == "x86_64", "fields");
}

static void test_feature() {
    FeatureAnnotation a; a.flag = "SSE4_2"; a.enabled = true;
    check(a.conceptType == "FeatureAnnotation", "conceptType");
    check(a.flag == "SSE4_2" && a.enabled, "fields");
}

static void test_original() {
    OriginalAnnotation a; a.sourceCode = "def foo(): pass"; a.sourceLanguage = "python";
    check(a.conceptType == "OriginalAnnotation", "conceptType");
    check(a.sourceCode == "def foo(): pass" && a.sourceLanguage == "python", "fields");
}

static void test_mapping() {
    MappingAnnotation a; a.history = {"python→whetstone", "whetstone→cpp"};
    check(a.conceptType == "MappingAnnotation", "conceptType");
    check(a.history.size() == 2, "history has 2 entries");
}

static void test_target_roundtrip() {
    auto* a = new TargetAnnotation(); a->id = "t1"; a->platform = "windows"; a->arch = "arm64";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rt = dynamic_cast<TargetAnnotation*>(r);
    check(rt && rt->platform == "windows" && rt->arch == "arm64", "TargetAnnotation roundtrip");
    delete a; delete r;
}

static void test_feature_roundtrip() {
    auto* a = new FeatureAnnotation(); a->id = "f1"; a->flag = "NEON"; a->enabled = false;
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rf = dynamic_cast<FeatureAnnotation*>(r);
    check(rf && rf->flag == "NEON" && !rf->enabled, "FeatureAnnotation roundtrip");
    delete a; delete r;
}

static void test_original_roundtrip() {
    auto* a = new OriginalAnnotation(); a->id = "o1";
    a->sourceCode = "print('hello')"; a->sourceLanguage = "python";
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* ro = dynamic_cast<OriginalAnnotation*>(r);
    check(ro && ro->sourceCode == "print('hello')" && ro->sourceLanguage == "python",
          "OriginalAnnotation roundtrip");
    delete a; delete r;
}

static void test_mapping_roundtrip() {
    auto* a = new MappingAnnotation(); a->id = "m1";
    a->history = {"step1", "step2", "step3"};
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rm = dynamic_cast<MappingAnnotation*>(r);
    check(rm && rm->history.size() == 3 && rm->history[2] == "step3", "MappingAnnotation roundtrip");
    delete a; delete r;
}

static void test_create_node_factory() {
    for (const auto& t : {"TargetAnnotation", "FeatureAnnotation",
                           "OriginalAnnotation", "MappingAnnotation"}) {
        ASTNode* n = createNode(t);
        check(n && n->conceptType == t, std::string("createNode(") + t + ")");
        delete n;
    }
}

static void test_compact_ast_target() {
    auto* f = new Function(); f->id = "f1"; f->name = "platform_init";
    auto* a = new TargetAnnotation(); a->platform = "linux"; a->arch = "x86_64";
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("target") && sem["target"]["platform"] == "linux", "target in compact AST");
    delete f;
}

static void test_compact_ast_original() {
    auto* f = new Function(); f->id = "f2"; f->name = "legacy";
    auto* a = new OriginalAnnotation(); a->sourceCode = "x"; a->sourceLanguage = "java";
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("original") && sem["original"]["lang"] == "java", "original in compact AST");
    delete f;
}

static void test_all_provenance_annotations() {
    auto* f = new Function(); f->id = "f3"; f->name = "full";
    f->addChild("annotations", [&]{ auto* a = new TargetAnnotation(); a->platform="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new FeatureAnnotation(); a->flag="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new OriginalAnnotation(); a->sourceCode="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new MappingAnnotation(); a->history={"x"}; return a; }());
    json sem = extractSemanticSummary(f);
    check(sem.contains("target") && sem.contains("feature") &&
          sem.contains("original") && sem.contains("mappingSteps"),
          "all 4 provenance annotations in summary");
    delete f;
}

int main() {
    std::cout << "=== Step 279: Platform & Provenance Annotations ===\n";
    test_target(); test_feature(); test_original(); test_mapping();
    test_target_roundtrip(); test_feature_roundtrip();
    test_original_roundtrip(); test_mapping_roundtrip();
    test_create_node_factory(); test_compact_ast_target();
    test_compact_ast_original(); test_all_provenance_annotations();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
