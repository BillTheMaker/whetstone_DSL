// Step 861: Deterministic fallback contract when hints unavailable (10 tests)
#include "graduation/DeterministicFallback.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(no_hints_uses_fallback);
    auto d=DeterministicFallback::apply("py->cpp","ctx",false);
    C(d.usedFallback,"fallback");P();}
void t2(){T(hints_available_no_fallback);
    auto d=DeterministicFallback::apply("py->cpp","ctx",true);
    C(!d.usedFallback,"no fallback");P();}
void t3(){T(pair_id_set);
    auto d=DeterministicFallback::apply("rust->go","ctx",false);
    C(d.pairId=="rust->go","pairId");P();}
void t4(){T(fallback_rule_set);
    auto d=DeterministicFallback::apply("py->cpp","ctx",false);
    C(!d.rule.empty(),"rule");P();}
void t5(){T(fallback_action_set);
    auto d=DeterministicFallback::apply("py->cpp","ctx",false);
    C(!d.action.empty(),"action");P();}
void t6(){T(fallback_reason_no_model);
    auto d=DeterministicFallback::apply("py->cpp","ctx",false);
    C(d.reason=="no_hint_model_available","reason");P();}
void t7(){T(hint_guided_rule);
    auto d=DeterministicFallback::apply("py->cpp","ctx",true);
    C(d.rule=="hint_guided","rule");P();}
void t8(){T(validate_valid);
    auto d=DeterministicFallback::apply("py->cpp","ctx",false);
    C(DeterministicFallback::validate(d),"valid");P();}
void t9(){T(validate_empty_pair_fails);
    FallbackDecision d; d.rule="r";
    std::string err;
    C(!DeterministicFallback::validate(d,&err)&&err=="pair_id_missing","error");P();}
void t10(){T(to_json_has_used_fallback);
    auto d=DeterministicFallback::apply("py->cpp","ctx",false);
    auto j=DeterministicFallback::toJson(d);
    C(j.contains("used_fallback"),"json");P();}

int main(){
    std::cout<<"Step 861: Deterministic fallback contract\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
