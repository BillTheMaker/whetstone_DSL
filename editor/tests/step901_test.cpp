// Step 901: Migration feasibility engine (8 tests)
#include "graduation/MigrationFeasibilityEngine.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(feasible_verdict);
    // riskScore=0.1 -> score=1-0.05=0.95 >= 0.6 -> feasible
    auto r=MigrationFeasibilityEngine::assess("p1","jvm11","cpython3",0.1f,true,true);
    C(r.verdict=="feasible","feas");P();}
void t2(){T(marginal_verdict);
    // riskScore=0.9 -> score=1-0.45=0.55, 0.35<=0.55<0.6 -> marginal
    auto r=MigrationFeasibilityEngine::assess("p1","jvm11","cpython3",0.9f,true,true);
    C(r.verdict=="marginal","marg");P();}
void t3(){T(infeasible_verdict);
    // riskScore=0.9 + both packs missing -> score=0.55-0.4=0.15 < 0.35 -> infeasible
    auto r=MigrationFeasibilityEngine::assess("p1","jvm11","cpython3",0.9f,false,false);
    C(r.verdict=="infeasible","inf");P();}
void t4(){T(pair_id_set);
    auto r=MigrationFeasibilityEngine::assess("myPair","jvm11","cpython3",0.1f,true,true);
    C(r.pairId=="myPair","id");P();}
void t5(){T(missing_source_pack_adds_blocker);
    auto r=MigrationFeasibilityEngine::assess("p1","jvm11","cpython3",0.1f,false,true);
    bool found=false;
    for(const auto& b:r.blockers) if(b.rfind("missing_pack",0)==0) found=true;
    C(found,"blocker");P();}
void t6(){T(missing_target_pack_adds_blocker);
    auto r=MigrationFeasibilityEngine::assess("p1","jvm11","cpython3",0.1f,true,false);
    bool found=false;
    for(const auto& b:r.blockers) if(b.rfind("missing_pack",0)==0) found=true;
    C(found,"blocker");P();}
void t7(){T(no_blockers_when_packs_available);
    auto r=MigrationFeasibilityEngine::assess("p1","jvm11","cpython3",0.1f,true,true);
    C(r.blockers.empty(),"no blockers");P();}
void t8(){T(score_positive);
    auto r=MigrationFeasibilityEngine::assess("p1","jvm11","cpython3",0.1f,true,true);
    C(r.score>0.0f,"score");P();}

int main(){
    std::cout<<"Step 901: Migration feasibility engine\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
