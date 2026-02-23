// Step 882: Starvation prevention mechanism (10 tests)
#include "graduation/StarvationPrevention.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(below_threshold_not_at_risk);
    auto e=StarvationPrevention::assess("py->cpp",3,5);
    C(!e.atRisk,"not risk");P();}
void t2(){T(at_threshold_at_risk);
    auto e=StarvationPrevention::assess("py->cpp",5,5);
    C(e.atRisk,"risk");P();}
void t3(){T(above_threshold_at_risk);
    auto e=StarvationPrevention::assess("py->cpp",8,5);
    C(e.atRisk,"risk");P();}
void t4(){T(at_risk_promoted);
    auto e=StarvationPrevention::assess("py->cpp",5,5);
    C(e.promoted,"promoted");P();}
void t5(){T(not_at_risk_not_promoted);
    auto e=StarvationPrevention::assess("py->cpp",2,5);
    C(!e.promoted,"not promoted");P();}
void t6(){T(pair_id_set);
    auto e=StarvationPrevention::assess("rust->go",3,5);
    C(e.pairId=="rust->go","pairId");P();}
void t7(){T(wait_cycles_stored);
    auto e=StarvationPrevention::assess("py->cpp",7,5);
    C(e.waitCycles==7,"cycles");P();}
void t8(){T(count_at_risk_zero);
    std::vector<StarvationRiskEntry> v={
        StarvationPrevention::assess("a",2,5),
        StarvationPrevention::assess("b",3,5)};
    C(StarvationPrevention::countAtRisk(v)==0,"zero");P();}
void t9(){T(count_at_risk_nonzero);
    std::vector<StarvationRiskEntry> v={
        StarvationPrevention::assess("a",6,5),
        StarvationPrevention::assess("b",2,5)};
    C(StarvationPrevention::countAtRisk(v)==1,"one");P();}
void t10(){T(to_json_has_at_risk);
    auto e=StarvationPrevention::assess("py->cpp",6,5);
    auto j=StarvationPrevention::toJson(e);
    C(j.contains("at_risk"),"json");P();}

int main(){
    std::cout<<"Step 882: Starvation prevention\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
