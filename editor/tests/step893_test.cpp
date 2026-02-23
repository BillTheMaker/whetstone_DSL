// Step 893: Runtime-aware raising policy selectors (8 tests)
#include "graduation/RuntimeRaisingPolicy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(no_pack_conservative);
    auto r=RuntimeRaisingPolicy::select("p1","jvm11",false,0.2f);
    C(r.policy=="conservative","cons");P();}
void t2(){T(no_pack_requires_review);
    auto r=RuntimeRaisingPolicy::select("p1","jvm11",false,0.2f);
    C(r.requiresReview,"review");P();}
void t3(){T(high_risk_strict);
    auto r=RuntimeRaisingPolicy::select("p1","jvm11",true,0.8f);
    C(r.policy=="strict","strict");P();}
void t4(){T(high_risk_requires_review);
    auto r=RuntimeRaisingPolicy::select("p1","jvm11",true,0.8f);
    C(r.requiresReview,"review");P();}
void t5(){T(medium_risk_relaxed);
    auto r=RuntimeRaisingPolicy::select("p1","jvm11",true,0.5f);
    C(r.policy=="relaxed","relaxed");P();}
void t6(){T(low_risk_relaxed);
    auto r=RuntimeRaisingPolicy::select("p1","jvm11",true,0.1f);
    C(r.policy=="relaxed","relaxed");P();}
void t7(){T(pair_id_set);
    auto r=RuntimeRaisingPolicy::select("myPair","jvm11",true,0.1f);
    C(r.pairId=="myPair","id");P();}
void t8(){T(to_json_has_policy);
    auto r=RuntimeRaisingPolicy::select("p1","jvm11",true,0.1f);
    auto j=RuntimeRaisingPolicy::toJson(r);
    C(j.contains("policy")&&j.contains("requires_review"),"json");P();}

int main(){
    std::cout<<"Step 893: Runtime-aware raising policy selectors\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
