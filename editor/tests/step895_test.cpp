// Step 895: Runtime compatibility risk report (8 tests)
#include "graduation/RuntimeCompatibilityRiskReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(both_packs_no_missing);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",true,true);
    C(!r.missingPackData,"no missing");P();}
void t2(){T(missing_source_pack_flagged);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",false,true);
    C(r.missingPackData,"missing");P();}
void t3(){T(missing_target_pack_flagged);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",true,false);
    C(r.missingPackData,"missing");P();}
void t4(){T(missing_pack_adds_risk_item);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",false,true);
    C(!r.riskItems.empty(),"item");P();}
void t5(){T(missing_pack_medium_risk_label);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",false,true);
    C(r.riskLabel=="medium","medium");P();}
void t6(){T(both_packs_no_risk_items);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",true,true);
    C(r.riskItems.empty(),"empty");P();}
void t7(){T(both_packs_low_risk_label);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",true,true);
    C(r.riskLabel=="low","low");P();}
void t8(){T(to_json_has_risk_label);
    auto r=RuntimeCompatibilityRiskReport::generate("p1","jvm11","cpython3",true,true);
    auto j=RuntimeCompatibilityRiskReport::toJson(r);
    C(j.contains("risk_label")&&j.contains("overall_risk"),"json");P();}

int main(){
    std::cout<<"Step 895: Runtime compatibility risk report\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
