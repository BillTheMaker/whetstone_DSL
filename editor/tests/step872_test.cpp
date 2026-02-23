// Step 872: Multi-plan alternative generator (10 tests)
#include "graduation/MultiPlanAlternativeGenerator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(generates_three_plans);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    C(plans.size()==3,"three");P();}
void t2(){T(has_cheap_plan);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    bool found=false; for(auto&p:plans)if(p.profile=="cheap")found=true;
    C(found,"cheap");P();}
void t3(){T(has_balanced_plan);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    bool found=false; for(auto&p:plans)if(p.profile=="balanced")found=true;
    C(found,"balanced");P();}
void t4(){T(has_rigorous_plan);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    bool found=false; for(auto&p:plans)if(p.profile=="rigorous")found=true;
    C(found,"rigorous");P();}
void t5(){T(cheap_costs_less_than_balanced);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    float cheap=0,bal=0;
    for(auto&p:plans){if(p.profile=="cheap")cheap=p.estimatedCost;if(p.profile=="balanced")bal=p.estimatedCost;}
    C(cheap<bal,"cost");P();}
void t6(){T(rigorous_costs_more_than_balanced);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    float rig=0,bal=0;
    for(auto&p:plans){if(p.profile=="rigorous")rig=p.estimatedCost;if(p.profile=="balanced")bal=p.estimatedCost;}
    C(rig>bal,"cost");P();}
void t7(){T(rigorous_requires_review);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    bool found=false;
    for(auto&p:plans)if(p.profile=="rigorous"&&p.requiresReview)found=true;
    C(found,"review");P();}
void t8(){T(cheap_no_review);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    bool ok=false;
    for(auto&p:plans)if(p.profile=="cheap"&&!p.requiresReview)ok=true;
    C(ok,"no review");P();}
void t9(){T(quality_increases_with_cost);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    float qc=0,qb=0,qr=0;
    for(auto&p:plans){if(p.profile=="cheap")qc=p.estimatedQuality;if(p.profile=="balanced")qb=p.estimatedQuality;if(p.profile=="rigorous")qr=p.estimatedQuality;}
    C(qc<qb&&qb<qr,"quality");P();}
void t10(){T(to_json_has_profile);
    auto plans=MultiPlanAlternativeGenerator::generate("py->cpp",0.5f);
    auto j=MultiPlanAlternativeGenerator::toJson(plans[0]);
    C(j.contains("profile"),"profile");P();}

int main(){
    std::cout<<"Step 872: Multi-plan alternative generator\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
