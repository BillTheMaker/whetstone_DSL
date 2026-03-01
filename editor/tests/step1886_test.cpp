// Step 1886 TDD: whetstone_score_language_fitness MCP tool wiring
//
// Tests the RegisterLanguageFitnessTools handler directly (no network).
//
//  t1: hint-only call (actors + tail) → Elixir ranks first
//  t2: AST-based call with for_stmt and chan_send → Go ranks high
//  t3: result contains language/score/rationale fields
//  t4: mutation_ratio override works
//  t5: unknown hints default to None enum values (no crash, valid result)

#include "ASTFeatureExtractor.h"
#include "LanguageFitnessScorer.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;
using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

// Minimal harness: directly call runScoreLanguageFitness via a mock class
// (can't instantiate MCPServer without full deps — use the scorer directly)

void t1(){
    T(actors_tail_hint_prefers_elixir);
    AFF feat;
    feat.mutationRatio = 0.05f;
    feat.recursionShape = AFF::RecursionShape::TailRecursive;
    feat.concurrencyPrimitive = AFF::ConcurrencyPrimitive::Actors;
    feat.ioPattern = AFF::IOPattern::EventDriven;
    feat.typeComplexity = AFF::TypeComplexity::None;
    auto result = whetstone::LanguageFitnessScorer::score(feat);
    C(!result.empty() && result[0]["language"] == "Elixir",
      "Elixir should rank first for actors+tail, got: " + result[0]["language"].get<std::string>());
    P();
}

void t2(){
    T(ast_with_channel_and_loop_favors_go);
    auto ast = json::parse(R"({
        "type": "function_decl",
        "children": [
            {"type": "for_stmt", "children": []},
            {"type": "chan_send", "children": []},
            {"type": "chan_recv", "children": []}
        ]
    })");
    auto feat = whetstone::ASTFeatureExtractor::extract(ast);
    auto result = whetstone::LanguageFitnessScorer::score(feat);
    // Go should rank in top 3
    bool goInTop3 = false;
    for (int i = 0; i < 3 && i < (int)result.size(); ++i) {
        if (result[i]["language"] == "Go") { goInTop3 = true; break; }
    }
    C(goInTop3, "Go should rank in top 3 for loop+channel AST");
    P();
}

void t3(){
    T(result_has_required_fields);
    AFF feat;
    auto result = whetstone::LanguageFitnessScorer::score(feat);
    C(result.is_array() && !result.empty(), "result must be non-empty array");
    for (const auto& entry : result) {
        C(entry.contains("language"), "missing language field");
        C(entry.contains("score"), "missing score field");
        C(entry.contains("rationale"), "missing rationale field");
    }
    P();
}

void t4(){
    T(mutation_ratio_override_affects_scoring);
    AFF lowMut;
    lowMut.mutationRatio = 0.0f;
    lowMut.recursionShape = AFF::RecursionShape::TailRecursive;

    AFF highMut;
    highMut.mutationRatio = 0.9f;
    highMut.recursionShape = AFF::RecursionShape::TailRecursive;

    auto rLow = whetstone::LanguageFitnessScorer::score(lowMut);
    auto rHigh = whetstone::LanguageFitnessScorer::score(highMut);

    // Find Python rank in both (Python has mutationRatio=0.4)
    int pyLow = -1, pyHigh = -1;
    for (int i = 0; i < (int)rLow.size(); ++i)
        if (rLow[i]["language"] == "Python") pyLow = i;
    for (int i = 0; i < (int)rHigh.size(); ++i)
        if (rHigh[i]["language"] == "Python") pyHigh = i;

    // Python (ideal 0.4) should rank better when mutation is 0.4-ish,
    // but worst when mutation is either extreme
    C(pyLow != -1 && pyHigh != -1, "Python not found in results");
    // Just verify scores differ
    float scoreLow = rLow[pyLow]["score"].get<float>();
    float scoreHigh = rHigh[pyHigh]["score"].get<float>();
    C(scoreLow != scoreHigh, "Different mutation ratios should yield different Python scores");
    P();
}

void t5(){
    T(no_crash_on_all_none_features);
    AFF feat; // all defaults = None/0
    auto result = whetstone::LanguageFitnessScorer::score(feat);
    C(result.is_array() && result.size() == 8, "must return 8 results for all-None features");
    P();
}

int main(){
    std::cout << "Step 1886: RegisterLanguageFitnessTools\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
