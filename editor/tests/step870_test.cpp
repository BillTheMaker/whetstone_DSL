// Step 870: Review effort estimator from ambiguity/risk packets (10 tests)
#include "graduation/ReviewEffortEstimator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(pair_id_set);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.3f,0.2f);
    C(p2.pairId=="py->cpp","pairId");P();}
void t2(){T(minutes_positive);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.5f,0.5f);
    C(p2.estimatedReviewMinutes>0,"minutes");P();}
void t3(){T(trivial_label);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.1f,0.1f);
    C(p2.complexityLabel=="trivial","trivial");P();}
void t4(){T(simple_label);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.3f,0.3f);
    C(p2.complexityLabel=="simple","simple");P();}
void t5(){T(moderate_label);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.5f,0.5f);
    C(p2.complexityLabel=="moderate","moderate");P();}
void t6(){T(complex_label);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.9f,0.9f);
    C(p2.complexityLabel=="complex","complex");P();}
void t7(){T(ambiguity_stored);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.4f,0.2f);
    C(p2.ambiguity==0.4f,"amb");P();}
void t8(){T(risk_stored);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.2f,0.6f);
    C(p2.riskScore==0.6f,"risk");P();}
void t9(){T(to_json_has_complexity);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.3f,0.2f);
    auto j=ReviewEffortEstimator::toJson(p2);
    C(j.contains("complexity_label"),"label");P();}
void t10(){T(to_json_has_minutes);
    auto p2=ReviewEffortEstimator::estimate("py->cpp",0.3f,0.2f);
    auto j=ReviewEffortEstimator::toJson(p2);
    C(j.contains("estimated_review_minutes"),"minutes");P();}

int main(){
    std::cout<<"Step 870: Review effort estimator\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
