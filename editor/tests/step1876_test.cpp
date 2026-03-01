// Step 1876 TDD: GR-013 — ParitySkewAnalyzer detects cross-language readiness asymmetry
//
// CrossLanguageConsistencyGate evaluates per-request but provides no skew tracking.
// ParitySkewAnalyzer adds batch analysis: given a set of language readiness results,
// compute skew (0=all ready, 1=none ready) and flag when skew > threshold.
//
//  t1: all languages ready → skew=0.0, belowThreshold=false
//  t2: half languages not ready → skew=0.5, belowThreshold=true (threshold=0.4)
//  t3: no languages ready → skew=1.0, notReadyLanguages has all
//  t4: empty language list → skew=0.0, totalCount=0
//  t5: batch evaluate class-decl across c++/python/rust → produces skew result

#include "ParitySkewAnalyzer.h"
#include <iostream>
#include <string>
#include <cmath>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static bool approx(float a, float b) { return std::fabs(a - b) < 0.01f; }

void t1(){
    T(all_ready_skew_zero);
    auto result = ParitySkewAnalyzer::analyze({
        {"python", true},
        {"cpp",    true},
        {"rust",   true}
    });
    C(approx(result.skew, 0.0f),
      "expected skew=0.0 when all languages ready, got " + std::to_string(result.skew));
    C(!result.belowThreshold, "should not be below threshold when skew=0");
    C(result.readyCount == 3, "expected readyCount=3");
    P();
}

void t2(){
    T(half_not_ready_skew_half);
    auto result = ParitySkewAnalyzer::analyze({
        {"python", true},
        {"cpp",    false},
        {"rust",   true},
        {"go",     false}
    }, 0.4f);
    C(approx(result.skew, 0.5f),
      "expected skew=0.5, got " + std::to_string(result.skew));
    C(result.belowThreshold, "skew=0.5 > threshold=0.4 → belowThreshold should be true");
    C(result.notReadyLanguages.size() == 2, "expected 2 not-ready languages");
    P();
}

void t3(){
    T(none_ready_skew_one);
    auto result = ParitySkewAnalyzer::analyze({
        {"python", false},
        {"cpp",    false}
    });
    C(approx(result.skew, 1.0f),
      "expected skew=1.0 when none ready, got " + std::to_string(result.skew));
    C(result.notReadyLanguages.size() == 2, "expected both in notReadyLanguages");
    P();
}

void t4(){
    T(empty_list_skew_zero);
    auto result = ParitySkewAnalyzer::analyze({});
    C(approx(result.skew, 0.0f), "expected skew=0.0 for empty list");
    C(result.totalCount == 0, "expected totalCount=0");
    C(!result.belowThreshold, "empty list should not be below threshold");
    P();
}

void t5(){
    T(batch_evaluate_construct_returns_skew_result);
    // Evaluate "class" construct with "add_method" operation across cpp/python/rust
    auto result = ParitySkewAnalyzer::evaluateConstruct(
        {"cpp", "python", "rust"}, "class", "add_method");
    // Don't assert specific values — gate results depend on adapter implementation.
    // Assert that the analysis ran and produced a valid result.
    C(result.totalCount == 3, "expected totalCount=3");
    C(result.skew >= 0.0f && result.skew <= 1.0f,
      "skew must be in [0,1], got " + std::to_string(result.skew));
    C(result.readyCount + static_cast<int>(result.notReadyLanguages.size()) == 3,
      "readyCount + notReady must equal total");
    P();
}

int main(){
    std::cout<<"Step 1876: GR-013 ParitySkewAnalyzer cross-language readiness asymmetry\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
