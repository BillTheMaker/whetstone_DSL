// Step 282: Strategy & Policy Annotations (12 tests)
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

static void test_policy() {
    PolicyAnnotation a;
    a.strictness = "high"; a.perf = "critical"; a.style = "idiomatic"; a.binaryStable = true;
    check(a.conceptType == "PolicyAnnotation", "conceptType");
    check(a.strictness == "high" && a.perf == "critical" &&
          a.style == "idiomatic" && a.binaryStable, "fields");
}

static void test_ambiguity() {
    AmbiguityAnnotation a; a.intent = "string vs bytes"; a.options = {"String", "Vec<u8>"};
    check(a.conceptType == "AmbiguityAnnotation", "conceptType");
    check(a.options.size() == 2, "options has 2 entries");
}

static void test_candidate_tradeoff() {
    CandidateAnnotation c; c.inferredTypes = {"i32", "i64", "f64"};
    check(c.inferredTypes.size() == 3, "CandidateAnnotation 3 types");
    TradeoffAnnotation t; t.reason = "perf vs safety"; t.safetyCost = "high"; t.perfCost = "low";
    check(t.reason == "perf vs safety", "TradeoffAnnotation reason");
}

static void test_choice_decision() {
    ChoiceAnnotation ch; ch.choiceId = "ch1"; ch.options = {"mutex", "rwlock", "atomic"};
    check(ch.choiceId == "ch1" && ch.options.size() == 3, "ChoiceAnnotation fields");
    DecisionAnnotation d; d.choiceId = "ch1"; d.selection = "atomic";
    d.author = "agent"; d.reason = "single-writer pattern";
    check(d.selection == "atomic" && d.author == "agent", "DecisionAnnotation fields");
}

static void test_policy_roundtrip() {
    auto* a = new PolicyAnnotation(); a->id = "p1";
    a->strictness = "low"; a->perf = "normal"; a->style = "literal"; a->binaryStable = false;
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rp = dynamic_cast<PolicyAnnotation*>(r);
    check(rp && rp->strictness == "low" && rp->perf == "normal" &&
          rp->style == "literal" && !rp->binaryStable, "PolicyAnnotation roundtrip");
    delete a; delete r;
}

static void test_ambiguity_roundtrip() {
    auto* a = new AmbiguityAnnotation(); a->id = "a1";
    a->intent = "type choice"; a->options = {"A", "B"};
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* ra = dynamic_cast<AmbiguityAnnotation*>(r);
    check(ra && ra->intent == "type choice" && ra->options.size() == 2, "AmbiguityAnnotation roundtrip");
    delete a; delete r;
}

static void test_choice_decision_roundtrip() {
    auto* ch = new ChoiceAnnotation(); ch->id = "ch1";
    ch->choiceId = "sync_strategy"; ch->options = {"mutex", "rwlock"};
    json jch = toJson(ch); ASTNode* rch = fromJson(jch);
    auto* rc = dynamic_cast<ChoiceAnnotation*>(rch);
    check(rc && rc->choiceId == "sync_strategy" && rc->options.size() == 2, "ChoiceAnnotation roundtrip");

    auto* d = new DecisionAnnotation(); d->id = "d1";
    d->choiceId = "sync_strategy"; d->selection = "rwlock"; d->author = "human"; d->reason = "read-heavy";
    json jd = toJson(d); ASTNode* rd = fromJson(jd);
    auto* rr = dynamic_cast<DecisionAnnotation*>(rd);
    check(rr && rr->selection == "rwlock" && rr->author == "human", "DecisionAnnotation roundtrip");
    delete ch; delete rch; delete d; delete rd;
}

static void test_create_node_factory() {
    for (const auto& t : {"PolicyAnnotation", "AmbiguityAnnotation", "CandidateAnnotation",
                           "TradeoffAnnotation", "ChoiceAnnotation", "DecisionAnnotation"}) {
        ASTNode* n = createNode(t);
        check(n && n->conceptType == t, std::string("createNode(") + t + ")");
        delete n;
    }
}

static void test_compact_ast_policy() {
    auto* f = new Function(); f->id = "f1"; f->name = "configure";
    auto* a = new PolicyAnnotation(); a->strictness = "high"; a->perf = "critical";
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("policy") && sem["policy"]["strictness"] == "high", "policy in compact AST");
    delete f;
}

static void test_compact_ast_decision() {
    auto* f = new Function(); f->id = "f2"; f->name = "decided";
    auto* a = new DecisionAnnotation(); a->choiceId = "c1"; a->selection = "optA";
    f->addChild("annotations", a);
    json sem = extractSemanticSummary(f);
    check(sem.contains("decision") && sem["decision"]["selection"] == "optA", "decision in compact AST");
    delete f;
}

static void test_all_policy_annotations() {
    auto* f = new Function(); f->id = "f3"; f->name = "full";
    f->addChild("annotations", [&]{ auto* a = new PolicyAnnotation(); a->strictness="high"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new AmbiguityAnnotation(); a->intent="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new CandidateAnnotation(); a->inferredTypes={"i32"}; return a; }());
    f->addChild("annotations", [&]{ auto* a = new TradeoffAnnotation(); a->reason="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new ChoiceAnnotation(); a->choiceId="x"; return a; }());
    f->addChild("annotations", [&]{ auto* a = new DecisionAnnotation(); a->choiceId="x"; return a; }());
    json sem = extractSemanticSummary(f);
    check(sem.contains("policy") && sem.contains("ambiguity") &&
          sem.contains("candidates") && sem.contains("tradeoff") &&
          sem.contains("choice") && sem.contains("decision"),
          "all 6 policy annotations in summary");
    delete f;
}

static void test_candidate_roundtrip() {
    auto* a = new CandidateAnnotation(); a->id = "c1";
    a->inferredTypes = {"String", "&str", "Vec<u8>"};
    json j = toJson(a); ASTNode* r = fromJson(j);
    auto* rc = dynamic_cast<CandidateAnnotation*>(r);
    check(rc && rc->inferredTypes.size() == 3, "CandidateAnnotation roundtrip");
    delete a; delete r;
}

int main() {
    std::cout << "=== Step 282: Strategy & Policy Annotations ===\n";
    test_policy(); test_ambiguity(); test_candidate_tradeoff();
    test_choice_decision(); test_policy_roundtrip();
    test_ambiguity_roundtrip(); test_choice_decision_roundtrip();
    test_create_node_factory(); test_compact_ast_policy();
    test_compact_ast_decision(); test_all_policy_annotations();
    test_candidate_roundtrip();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
