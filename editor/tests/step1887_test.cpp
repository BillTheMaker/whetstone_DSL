// Step 1887: Sprint 271 Integration — end-to-end spec section → features → scores → ranked list
//
//  t1: supervision-tree spec section → Elixir ranked #1
//  t2: data-crunching AST → Rust in top-2
//  t3: event-loop web server → TypeScript or Go in top-3
//  t4: pure math spec → Haskell or Lisp in top-2
//  t5: profile count is 8 and result is sorted; rationale non-empty on mismatched profiles

#include "ASTFeatureExtractor.h"
#include "LanguageIdiomProfile.h"
#include "LanguageFitnessScorer.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <algorithm>

using json = nlohmann::json;
using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static int rankOf(const json& arr, const std::string& lang) {
    for (int i = 0; i < (int)arr.size(); ++i)
        if (arr[i]["language"] == lang) return i;
    return -1;
}

void t1(){
    T(supervision_tree_spec_elixir_first);
    // Supervision tree: actors + event-driven + tail recursive
    auto ast = json::parse(R"({
        "type": "module",
        "children": [
            {"type": "actor_spawn", "children": []},
            {"type": "receive_msg", "children": []},
            {"type": "return_stmt", "children": [
                {"type": "call_expr", "children": []}
            ]},
            {"type": "emit_event", "children": []}
        ]
    })");
    auto feat = whetstone::ASTFeatureExtractor::extract(ast);
    C(feat.concurrencyPrimitive == AFF::ConcurrencyPrimitive::Actors,
      "actors not detected");
    C(feat.ioPattern == AFF::IOPattern::EventDriven,
      "event-driven not detected");
    auto ranked = whetstone::LanguageFitnessScorer::score(feat);
    C(ranked[0]["language"] == "Elixir",
      "Elixir should rank #1, got: " + ranked[0]["language"].get<std::string>());
    P();
}

void t2(){
    T(data_crunching_rust_in_top2);
    // Data crunch: shared memory, tree-recursive calls, low mutation, async, generics
    AFF feat;
    feat.mutationRatio = 0.1f;
    feat.recursionShape = AFF::RecursionShape::TreeRecursive;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::SharedMemory;
    feat.ioPattern = AFF::IOPattern::Async;
    feat.typeComplexity = AFF::TypeComplexity::SimpleGenerics;
    auto ranked = whetstone::LanguageFitnessScorer::score(feat);
    int rustRank = rankOf(ranked, "Rust");
    C(rustRank != -1 && rustRank < 2, "Rust should be in top-2 for data-crunch spec, rank=" + std::to_string(rustRank));
    P();
}

void t3(){
    T(event_loop_ts_or_go_top3);
    // Event-loop web server: FlatLoop, async I/O, no concurrency, simple generics
    AFF feat;
    feat.mutationRatio = 0.3f;
    feat.recursionShape = AFF::RecursionShape::FlatLoop;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::None;
    feat.ioPattern = AFF::IOPattern::Async;
    feat.typeComplexity = AFF::TypeComplexity::SimpleGenerics;
    auto ranked = whetstone::LanguageFitnessScorer::score(feat);
    int tsRank = rankOf(ranked, "TypeScript");
    int goRank = rankOf(ranked, "Go");
    bool eitherTop3 = (tsRank != -1 && tsRank < 3) || (goRank != -1 && goRank < 3);
    C(eitherTop3, "TypeScript or Go should be in top-3 for event-loop spec, ts=" + std::to_string(tsRank) + " go=" + std::to_string(goRank));
    P();
}

void t4(){
    T(pure_math_haskell_or_lisp_top2);
    // Pure math: no mutation, tail recursive, no concurrency, no I/O, possibly dependent types
    AFF feat;
    feat.mutationRatio = 0.0f;
    feat.recursionShape = AFF::RecursionShape::TailRecursive;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::None;
    feat.ioPattern = AFF::IOPattern::None;
    feat.typeComplexity = AFF::TypeComplexity::None;
    auto ranked = whetstone::LanguageFitnessScorer::score(feat);
    int haskellRank = rankOf(ranked, "Haskell");
    int lispRank = rankOf(ranked, "Lisp");
    bool eitherTop2 = (haskellRank != -1 && haskellRank < 2) || (lispRank != -1 && lispRank < 2);
    C(eitherTop2, "Haskell or Lisp should be in top-2 for pure math, haskell=" + std::to_string(haskellRank) + " lisp=" + std::to_string(lispRank));
    P();
}

void t5(){
    T(profile_count_sorted_rationale_non_empty_on_mismatch);
    AFF feat;
    feat.mutationRatio = 0.5f;  // unusual value
    feat.recursionShape = AFF::RecursionShape::TreeRecursive;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::Actors;
    feat.ioPattern = AFF::IOPattern::Blocking;
    feat.typeComplexity = AFF::TypeComplexity::DependentTypes;
    auto ranked = whetstone::LanguageFitnessScorer::score(feat);
    C(ranked.size() == 8, "must return 8 profiles, got " + std::to_string(ranked.size()));
    // Verify sorted descending
    for (size_t i = 1; i < ranked.size(); ++i) {
        float prev = ranked[i-1]["score"].get<float>();
        float curr = ranked[i]["score"].get<float>();
        C(prev >= curr, "not sorted at index " + std::to_string(i));
    }
    // At least some entries should have non-empty rationale (since the feat is unusual)
    int rationales = 0;
    for (const auto& e : ranked) {
        if (!e["rationale"].get<std::string>().empty() &&
            e["rationale"].get<std::string>() != "ideal match") ++rationales;
    }
    C(rationales > 0, "expected some non-empty rationale for mismatched features");
    P();
}

int main(){
    std::cout << "Step 1887: Sprint 271 Integration\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
