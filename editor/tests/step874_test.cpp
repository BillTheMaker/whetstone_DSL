// Step 874: Cost-vs-quality tradeoff report generator (8 tests)
#include "graduation/CostQualityTradeoffReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(pair_id_set);
    auto r=CostQualityTradeoffReport::generate("py->cpp",
        {{"cheap",0.2f},{"rigorous",0.8f}},{{"cheap",0.6f},{"rigorous",0.95f}});
    C(r.pairId=="py->cpp","pairId");P();}
void t2(){T(entries_count);
    auto r=CostQualityTradeoffReport::generate("py->cpp",
        {{"cheap",0.2f},{"balanced",0.5f}},{{"cheap",0.6f},{"balanced",0.8f}});
    C(r.entries.size()==2,"size");P();}
void t3(){T(recommended_profile_set);
    auto r=CostQualityTradeoffReport::generate("py->cpp",
        {{"cheap",0.2f},{"balanced",0.5f}},{{"cheap",0.6f},{"balanced",0.8f}});
    C(!r.recommendedProfile.empty(),"rec");P();}
void t4(){T(efficiency_computed);
    auto r=CostQualityTradeoffReport::generate("py->cpp",
        {{"cheap",0.2f}},{{"cheap",0.6f}});
    C(r.entries[0].efficiency>0.0f,"eff");P();}
void t5(){T(best_efficiency_recommended);
    // cheap: 0.6/0.2 = 3.0, balanced: 0.8/0.5 = 1.6 → cheap wins
    auto r=CostQualityTradeoffReport::generate("py->cpp",
        {{"cheap",0.2f},{"balanced",0.5f}},{{"cheap",0.6f},{"balanced",0.8f}});
    C(r.recommendedProfile=="cheap","cheap");P();}
void t6(){T(to_json_has_entries);
    auto r=CostQualityTradeoffReport::generate("py->cpp",
        {{"cheap",0.2f}},{{"cheap",0.6f}});
    auto j=CostQualityTradeoffReport::toJson(r);
    C(j["entries"].is_array(),"entries");P();}
void t7(){T(to_json_has_recommended);
    auto r=CostQualityTradeoffReport::generate("py->cpp",
        {{"cheap",0.2f}},{{"cheap",0.6f}});
    auto j=CostQualityTradeoffReport::toJson(r);
    C(j.contains("recommended_profile"),"rec");P();}
void t8(){T(empty_plans_empty_entries);
    auto r=CostQualityTradeoffReport::generate("py->cpp",{},{});
    C(r.entries.empty(),"empty");P();}

int main(){
    std::cout<<"Step 874: Cost-vs-quality tradeoff report\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
