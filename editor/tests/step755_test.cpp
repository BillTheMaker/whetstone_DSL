// Step 755: Dynamic->static risk classifier (8 tests)
#include "dynamic/DynamicRiskClassifier.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(low_without_risk);auto k=DynamicRiskClassifier::classify(0,false,DynamicStrictnessPolicyEngine::forMode("strict"));C(k.level=="low","lvl");P();}
void t2(){T(medium_with_dispatch);auto k=DynamicRiskClassifier::classify(1,false,DynamicStrictnessPolicyEngine::forMode("strict"));C(k.level=="medium","lvl");P();}
void t3(){T(medium_with_reflection);auto k=DynamicRiskClassifier::classify(0,true,DynamicStrictnessPolicyEngine::forMode("strict"));C(k.level=="medium","lvl");P();}
void t4(){T(high_with_dispatch_and_reflection);auto k=DynamicRiskClassifier::classify(1,true,DynamicStrictnessPolicyEngine::forMode("strict"));C(k.level=="high","lvl");P();}
void t5(){T(reasons_non_empty_on_risk);auto k=DynamicRiskClassifier::classify(1,false,DynamicStrictnessPolicyEngine::forMode("strict"));C(!k.reasons.empty(),"reasons");P();}
void t6(){T(json_shape);auto j=DynamicRiskClassifier::toJson(DynamicRiskClassifier::classify(1,true,DynamicStrictnessPolicyEngine::forMode("strict")));C(j.contains("level")&&j.contains("reasons"),"shape");P();}
void t7(){T(machine_readable);auto j=DynamicRiskClassifier::toJson(DynamicRiskClassifier::classify(1,true,DynamicStrictnessPolicyEngine::forMode("strict")));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=DynamicRiskClassifier::toJson(DynamicRiskClassifier::classify(1,true,DynamicStrictnessPolicyEngine::forMode("strict"))).dump();auto b=DynamicRiskClassifier::toJson(DynamicRiskClassifier::classify(1,true,DynamicStrictnessPolicyEngine::forMode("strict"))).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 755: DynamicRiskClassifier\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
