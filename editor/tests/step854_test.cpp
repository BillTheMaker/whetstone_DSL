// Step 854: RemediationGenerator tests (8 tests)
#include "graduation/RemediationGenerator.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(T001_action);
    auto r=RemediationGenerator::generate("T001","high");
    C(r.action=="update_semantic_adapter","action");P();}

void t2(){T(T004_action);
    auto r=RemediationGenerator::generate("T004","high");
    C(r.action=="add_ownership_annotations","action");P();}

void t3(){T(other_code_generic_fix);
    auto r=RemediationGenerator::generate("T002","medium");
    C(r.action=="review_and_patch","generic fix");P();}

void t4(){T(priority_stored);
    auto r=RemediationGenerator::generate("T001","critical");
    C(r.priority=="critical","priority");P();}

void t5(){T(code_stored);
    auto r=RemediationGenerator::generate("T003","medium");
    C(r.code=="T003","code");P();}

void t6(){T(estimatedEffort_T001);
    auto r=RemediationGenerator::generate("T001","high");
    C(r.estimatedEffort>0,"effort>0");P();}

void t7(){T(description_not_empty);
    auto r=RemediationGenerator::generate("T001","high");
    C(!r.description.empty(),"description");P();}

void t8(){T(toJson_output);
    auto r=RemediationGenerator::generate("T001","high");
    auto j=RemediationGenerator::toJson(r);
    C(j.contains("code")&&j.contains("action"),"keys");P();}

int main(){
    std::cout<<"Step 854: RemediationGenerator\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
