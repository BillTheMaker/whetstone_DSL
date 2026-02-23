// Step 859: Hint feature extraction from decision ledgers (12 tests)
#include "graduation/HintFeatureExtractor.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

void t1() { T(empty_decisions_empty_features);
    auto r = HintFeatureExtractor::extract("py->cpp", {});
    C(r.empty(), "empty"); P(); }
void t2() { T(one_decision_one_feature);
    nlohmann::json d = {{"decision","approved"},{"rationale","looks good"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d});
    C(r.size()==1, "size"); P(); }
void t3() { T(feature_id_sequential);
    nlohmann::json d1={{"decision","approved"}}, d2={{"decision","rejected"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d1,d2});
    C(r[0].featureId=="HF-1" && r[1].featureId=="HF-2", "ids"); P(); }
void t4() { T(pair_id_set);
    nlohmann::json d={{"decision","modified"}};
    auto r = HintFeatureExtractor::extract("rust->go", {d});
    C(r[0].pairId=="rust->go", "pair"); P(); }
void t5() { T(source_is_decision_ledger);
    nlohmann::json d={{"decision","approved"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d});
    C(r[0].source=="decision_ledger", "source"); P(); }
void t6() { T(attribute_from_decision_field);
    nlohmann::json d={{"decision","approved"},{"rationale","ok"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d});
    C(r[0].attribute=="approved", "attr"); P(); }
void t7() { T(value_from_rationale_field);
    nlohmann::json d={{"decision","approved"},{"rationale","looks good"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d});
    C(r[0].value=="looks good", "value"); P(); }
void t8() { T(default_weight_one);
    nlohmann::json d={{"decision","approved"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d});
    C(r[0].weight==1.0f, "weight"); P(); }
void t9() { T(multiple_decisions);
    std::vector<nlohmann::json> decisions;
    for (int i=0;i<5;++i) decisions.push_back({{"decision","approved"}});
    auto r = HintFeatureExtractor::extract("py->cpp", decisions);
    C(r.size()==5, "count"); P(); }
void t10() { T(to_json_has_feature_id);
    nlohmann::json d={{"decision","approved"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d});
    auto j = HintFeatureExtractor::toJson(r[0]);
    C(j.contains("feature_id"), "json"); P(); }
void t11() { T(to_json_has_pair_id);
    nlohmann::json d={{"decision","approved"}};
    auto r = HintFeatureExtractor::extract("rust->cpp", {d});
    auto j = HintFeatureExtractor::toJson(r[0]);
    C(j.value("pair_id","")=="rust->cpp", "pair_id"); P(); }
void t12() { T(to_json_has_source);
    nlohmann::json d={{"decision","rejected"}};
    auto r = HintFeatureExtractor::extract("py->cpp", {d});
    auto j = HintFeatureExtractor::toJson(r[0]);
    C(j.contains("source"), "source"); P(); }

int main() {
    std::cout << "Step 859: Hint feature extraction from decision ledgers\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f?1:0;
}
