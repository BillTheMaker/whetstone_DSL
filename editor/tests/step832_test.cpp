// Step 832: ReleaseReadinessScoreboard tests (10 tests)
#include "graduation/ReleaseReadinessScoreboard.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(ready_at_90_80_80);
    auto s=ReleaseReadinessScoreboard::compute("py->cpp",90,80,80);
    C(s.releaseReady,"releaseReady");P();}

void t2(){T(not_ready_at_60_60_60);
    auto s=ReleaseReadinessScoreboard::compute("py->cpp",60,60,60);
    C(!s.releaseReady,"not ready");P();}

void t3(){T(tier_stable_at_95);
    auto s=ReleaseReadinessScoreboard::compute("py->cpp",95,95,95);
    C(s.tier=="stable","tier stable");P();}

void t4(){T(tier_beta_at_75);
    auto s=ReleaseReadinessScoreboard::compute("py->cpp",75,75,75);
    C(s.tier=="beta","tier beta");P();}

void t5(){T(rank_order);
    std::vector<ReadinessScore> scores={
        ReleaseReadinessScoreboard::compute("a",50,50,50),
        ReleaseReadinessScoreboard::compute("b",90,90,90)};
    auto ranked=ReleaseReadinessScoreboard::rank(scores);
    C(ranked[0].pairId=="b","first is highest");P();}

void t6(){T(overallScore_sum_div_3);
    auto s=ReleaseReadinessScoreboard::compute("py->cpp",90,60,30);
    C(s.overallScore==60,"overallScore=60");P();}

void t7(){T(all_three_inputs_matter);
    auto s1=ReleaseReadinessScoreboard::compute("py->cpp",100,100,100);
    auto s2=ReleaseReadinessScoreboard::compute("py->cpp",0,0,0);
    C(s1.overallScore!=s2.overallScore,"different scores");P();}

void t8(){T(pairId_preserved);
    auto s=ReleaseReadinessScoreboard::compute("rust->cpp",80,80,80);
    C(s.pairId=="rust->cpp","pairId");P();}

void t9(){T(experimental_tier_below_70);
    auto s=ReleaseReadinessScoreboard::compute("py->cpp",60,60,60);
    C(s.tier=="experimental","experimental");P();}

void t10(){T(toJson_has_keys);
    auto s=ReleaseReadinessScoreboard::compute("py->cpp",80,80,80);
    auto j=ReleaseReadinessScoreboard::toJson(s);
    C(j.contains("pair_id")&&j.contains("release_ready"),"keys");P();}

int main(){
    std::cout<<"Step 832: ReleaseReadinessScoreboard\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
