// Step 867: Hint safety guardrails and audit fields (8 tests)
#include "graduation/HintSafetyGuardrails.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(check_passes_valid);
    auto r=HintSafetyGuardrails::check("H-1",0.8f,true,true);
    C(r.passed,"passed");P();}
void t2(){T(check_fails_authoritative);
    auto r=HintSafetyGuardrails::check("H-1",0.8f,false,true);
    C(!r.passed,"failed");P();}
void t3(){T(check_fails_no_deterministic_policy);
    auto r=HintSafetyGuardrails::check("H-1",0.8f,true,false);
    C(!r.passed,"failed");P();}
void t4(){T(violation_in_list);
    auto r=HintSafetyGuardrails::check("H-1",0.8f,false,true);
    C(!r.violations.empty(),"violations");P();}
void t5(){T(hint_id_set);
    auto r=HintSafetyGuardrails::check("H-99",0.8f,true,true);
    C(r.hintId=="H-99","id");P();}
void t6(){T(audit_record_from_check);
    auto c=HintSafetyGuardrails::check("H-1",0.8f,true,true);
    auto a=HintSafetyGuardrails::audit("H-1","py->cpp",c);
    C(a.guardPassed,"passed");P();}
void t7(){T(audit_guard_reason_on_fail);
    auto c=HintSafetyGuardrails::check("H-1",0.8f,false,true);
    auto a=HintSafetyGuardrails::audit("H-1","py->cpp",c);
    C(!a.guardReason.empty(),"reason");P();}
void t8(){T(to_json_has_guard_passed);
    auto c=HintSafetyGuardrails::check("H-1",0.8f,true,true);
    auto a=HintSafetyGuardrails::audit("H-1","py->cpp",c);
    auto j=HintSafetyGuardrails::toJson(a);
    C(j.contains("guard_passed"),"json");P();}

int main(){
    std::cout<<"Step 867: Hint safety guardrails\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
