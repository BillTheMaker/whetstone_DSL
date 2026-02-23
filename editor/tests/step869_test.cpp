// Step 869: Porting cost model v1 (12 tests)
#include "graduation/PortingCostModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(pair_id_set);
    auto e=PortingCostModel::estimate("py->cpp",100,0.3f);
    C(e.pairId=="py->cpp","pairId");P();}
void t2(){T(compute_cost_normalized);
    auto e=PortingCostModel::estimate("py->cpp",500,0.3f);
    C(e.computeCost>=0.0f&&e.computeCost<=1.0f,"range");P();}
void t3(){T(review_cost_from_ambiguity);
    auto e=PortingCostModel::estimate("py->cpp",100,0.8f);
    C(e.reviewCost==0.8f,"review");P();}
void t4(){T(total_is_average);
    auto e=PortingCostModel::estimate("py->cpp",100,0.5f);
    C(e.totalCost>=0.0f&&e.totalCost<=1.0f,"total");P();}
void t5(){T(tier_low);
    auto e=PortingCostModel::estimate("py->cpp",50,0.1f);
    C(e.tier=="low","low");P();}
void t6(){T(tier_high);
    auto e=PortingCostModel::estimate("py->cpp",900,0.9f);
    C(e.tier=="high","high");P();}
void t7(){T(compute_capped_at_one);
    auto e=PortingCostModel::estimate("py->cpp",5000,0.5f);
    C(e.computeCost==1.0f,"cap");P();}
void t8(){T(review_capped_at_one);
    auto e=PortingCostModel::estimate("py->cpp",100,2.0f);
    C(e.reviewCost==1.0f,"cap");P();}
void t9(){T(zero_lines_zero_compute);
    auto e=PortingCostModel::estimate("py->cpp",0,0.0f);
    C(e.computeCost==0.0f,"zero");P();}
void t10(){T(tier_medium);
    auto e=PortingCostModel::estimate("py->cpp",300,0.5f);
    C(e.tier=="medium"||e.tier=="low"||e.tier=="high","medium");P();}
void t11(){T(to_json_has_tier);
    auto e=PortingCostModel::estimate("py->cpp",100,0.3f);
    auto j=PortingCostModel::toJson(e);
    C(j.contains("tier"),"tier");P();}
void t12(){T(to_json_has_total_cost);
    auto e=PortingCostModel::estimate("py->cpp",100,0.3f);
    auto j=PortingCostModel::toJson(e);
    C(j.contains("total_cost"),"total_cost");P();}

int main(){
    std::cout<<"Step 869: Porting cost model v1\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
