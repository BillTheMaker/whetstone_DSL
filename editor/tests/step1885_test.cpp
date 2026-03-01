// Step 1885 TDD: LanguageFitnessScorer — score AST features against language profiles
//
//  t1: data-parallel reduction features → Rust ranks higher than Python and Lisp
//  t2: supervision tree with restart → Elixir ranks #1
//  t3: result is a JSON array sorted descending by score
//  t4: perfect match to Go profile → Go scores >= 75
//  t5: Haskell profile match (mutation=0, tail-recursive, no concurrency) → Haskell ranks #1

#include "LanguageFitnessScorer.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static std::string langAt(const nlohmann::json& arr, int idx){
    return arr[idx]["language"].get<std::string>();
}
static float scoreAt(const nlohmann::json& arr, int idx){
    return arr[idx]["score"].get<float>();
}
static int rankOf(const nlohmann::json& arr, const std::string& lang){
    for (int i = 0; i < (int)arr.size(); ++i)
        if (arr[i]["language"].get<std::string>() == lang) return i;
    return -1;
}

void t1(){
    T(data_parallel_reduction_prefers_rust);
    // Data-parallel: low mutation, tree-recursive, shared memory, async, simple generics → Rust ideal
    AFF feat;
    feat.mutationRatio = 0.15f;
    feat.recursionShape = AFF::RecursionShape::TreeRecursive;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::SharedMemory;
    feat.ioPattern = AFF::IOPattern::Async;
    feat.typeComplexity = AFF::TypeComplexity::SimpleGenerics;

    auto result = whetstone::LanguageFitnessScorer::score(feat);
    int rustRank = rankOf(result, "Rust");
    int pythonRank = rankOf(result, "Python");
    int lispRank = rankOf(result, "Lisp");
    C(rustRank != -1 && pythonRank != -1 && lispRank != -1, "missing language in results");
    C(rustRank < pythonRank, "Rust should rank higher than Python for data-parallel reduction, rust=" + std::to_string(rustRank) + " python=" + std::to_string(pythonRank));
    C(rustRank < lispRank, "Rust should rank higher than Lisp for data-parallel reduction, rust=" + std::to_string(rustRank) + " lisp=" + std::to_string(lispRank));
    P();
}

void t2(){
    T(supervision_tree_prefers_elixir);
    // Supervision tree: tail-recursive, actors, event-driven, low mutation → Elixir ideal
    AFF feat;
    feat.mutationRatio = 0.05f;
    feat.recursionShape = AFF::RecursionShape::TailRecursive;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::Actors;
    feat.ioPattern = AFF::IOPattern::EventDriven;
    feat.typeComplexity = AFF::TypeComplexity::None;

    auto result = whetstone::LanguageFitnessScorer::score(feat);
    C(!result.empty(), "result must not be empty");
    C(langAt(result, 0) == "Elixir",
      "Elixir should rank #1 for supervision tree, got: " + langAt(result, 0));
    P();
}

void t3(){
    T(result_is_sorted_json_array);
    AFF feat;
    feat.mutationRatio = 0.3f;
    feat.recursionShape = AFF::RecursionShape::FlatLoop;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::None;
    feat.ioPattern = AFF::IOPattern::Blocking;
    feat.typeComplexity = AFF::TypeComplexity::None;

    auto result = whetstone::LanguageFitnessScorer::score(feat);
    C(result.is_array(), "result must be a JSON array");
    C(result.size() == 8, "must have 8 entries, got " + std::to_string(result.size()));
    // Verify sorted descending
    for (size_t i = 1; i < result.size(); ++i) {
        float prev = scoreAt(result, (int)i-1);
        float curr = scoreAt(result, (int)i);
        C(prev >= curr, "array not sorted descending at index " + std::to_string(i));
    }
    // Verify all have required fields
    for (const auto& entry : result) {
        C(entry.contains("language") && entry.contains("score") && entry.contains("rationale"),
          "entry missing required fields");
    }
    P();
}

void t4(){
    T(go_profile_match_scores_high);
    // Go ideal: FlatLoop, Channels, Blocking, mutation=0.3, no type complexity
    AFF feat;
    feat.mutationRatio = 0.3f;
    feat.recursionShape = AFF::RecursionShape::FlatLoop;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::Channels;
    feat.ioPattern = AFF::IOPattern::Blocking;
    feat.typeComplexity = AFF::TypeComplexity::None;

    auto result = whetstone::LanguageFitnessScorer::score(feat);
    int goRank = rankOf(result, "Go");
    float goScore = scoreAt(result, goRank);
    C(goScore >= 75.0f, "Go perfect match should score >= 75, got " + std::to_string(goScore));
    C(goRank == 0, "Go should rank #1 for Go-ideal features, rank=" + std::to_string(goRank));
    P();
}

void t5(){
    T(haskell_ideal_ranks_first);
    // Haskell ideal: TailRecursive, None concurrency, None IO, mutation=0, DependentTypes
    AFF feat;
    feat.mutationRatio = 0.0f;
    feat.recursionShape = AFF::RecursionShape::TailRecursive;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::None;
    feat.ioPattern = AFF::IOPattern::None;
    feat.typeComplexity = AFF::TypeComplexity::DependentTypes;

    auto result = whetstone::LanguageFitnessScorer::score(feat);
    C(langAt(result, 0) == "Haskell",
      "Haskell should rank #1 for Haskell-ideal features, got: " + langAt(result, 0));
    P();
}

int main(){
    std::cout << "Step 1885: LanguageFitnessScorer\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
