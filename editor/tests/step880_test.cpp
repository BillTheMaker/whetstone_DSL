// Step 880: Priority policy (10 tests)
#include "graduation/UpgradePriorityPolicy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(critical_level_4);
    auto r=UpgradePriorityPolicy::classify("py->cpp","critical");
    C(r.priorityLevel==4,"level");P();}
void t2(){T(revenue_level_3);
    auto r=UpgradePriorityPolicy::classify("py->cpp","revenue");
    C(r.priorityLevel==3,"level");P();}
void t3(){T(coverage_level_2);
    auto r=UpgradePriorityPolicy::classify("py->cpp","coverage");
    C(r.priorityLevel==2,"level");P();}
void t4(){T(experimental_level_1);
    auto r=UpgradePriorityPolicy::classify("py->cpp","experimental");
    C(r.priorityLevel==1,"level");P();}
void t5(){T(critical_preempts_experimental);
    auto r=UpgradePriorityPolicy::classify("py->cpp","critical");
    C(r.preemptsExperimental,"preempt");P();}
void t6(){T(revenue_no_preempt);
    auto r=UpgradePriorityPolicy::classify("py->cpp","revenue");
    C(!r.preemptsExperimental,"no preempt");P();}
void t7(){T(unknown_defaults_experimental);
    auto r=UpgradePriorityPolicy::classify("py->cpp","unknown");
    C(r.priorityClass=="experimental","default");P();}
void t8(){T(pair_id_stored);
    auto r=UpgradePriorityPolicy::classify("rust->go","critical");
    C(r.pairId=="rust->go","pairId");P();}
void t9(){T(validate_known_class);
    C(UpgradePriorityPolicy::validate("critical"),"valid");P();}
void t10(){T(validate_unknown_class_fails);
    C(!UpgradePriorityPolicy::validate("unknown"),"invalid");P();}

int main(){
    std::cout<<"Step 880: Priority policy\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
