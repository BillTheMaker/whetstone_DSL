// Step 871: Compute/runtime cost estimator from prior runs (10 tests)
#include "graduation/ComputeCostEstimator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(no_history_no_data);
    auto e=ComputeCostEstimator::estimate("py->cpp",{});
    C(!e.hasHistory,"no history");P();}
void t2(){T(with_history_has_data);
    auto e=ComputeCostEstimator::estimate("py->cpp",{{"py->cpp",100,500}});
    C(e.hasHistory,"history");P();}
void t3(){T(pair_id_set);
    auto e=ComputeCostEstimator::estimate("rust->go",{});
    C(e.pairId=="rust->go","pairId");P();}
void t4(){T(sample_count_correct);
    std::vector<PriorRunRecord> h={{"py->cpp",100,500},{"py->cpp",200,600}};
    auto e=ComputeCostEstimator::estimate("py->cpp",h);
    C(e.sampleCount==2,"count");P();}
void t5(){T(average_duration);
    std::vector<PriorRunRecord> h={{"py->cpp",100,500},{"py->cpp",200,600}};
    auto e=ComputeCostEstimator::estimate("py->cpp",h);
    C(e.estimatedDurationMs==150.0f,"avg");P();}
void t6(){T(average_tokens);
    std::vector<PriorRunRecord> h={{"py->cpp",100,500},{"py->cpp",200,600}};
    auto e=ComputeCostEstimator::estimate("py->cpp",h);
    C(e.estimatedTokens==550.0f,"tokens");P();}
void t7(){T(single_run);
    auto e=ComputeCostEstimator::estimate("py->cpp",{{"py->cpp",300,900}});
    C(e.estimatedDurationMs==300.0f,"single");P();}
void t8(){T(zero_history_zero_estimate);
    auto e=ComputeCostEstimator::estimate("py->cpp",{});
    C(e.estimatedDurationMs==0.0f,"zero");P();}
void t9(){T(to_json_has_history);
    auto e=ComputeCostEstimator::estimate("py->cpp",{{"py->cpp",100,500}});
    auto j=ComputeCostEstimator::toJson(e);
    C(j.contains("has_history"),"json");P();}
void t10(){T(to_json_has_sample_count);
    auto e=ComputeCostEstimator::estimate("py->cpp",{{"py->cpp",100,500}});
    auto j=ComputeCostEstimator::toJson(e);
    C(j.contains("sample_count"),"count");P();}

int main(){
    std::cout<<"Step 871: Compute cost estimator\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
