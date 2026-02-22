// Step 844: AutoBlockRuleEngine tests (8 tests)
#include "graduation/AutoBlockRuleEngine.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(no_rules_not_blocked);
    AutoBlockRuleEngine eng;
    auto d=eng.evaluate("py->cpp","stable","cert_fail");
    C(!d.blocked,"not blocked");P();}

void t2(){T(matching_rule_blocks);
    AutoBlockRuleEngine eng;
    eng.addRule({"R1","stable","cert_fail",true});
    auto d=eng.evaluate("py->cpp","stable","cert_fail");
    C(d.blocked,"blocked");P();}

void t3(){T(disabled_rule_no_block);
    AutoBlockRuleEngine eng;
    eng.addRule({"R1","stable","cert_fail",false});
    auto d=eng.evaluate("py->cpp","stable","cert_fail");
    C(!d.blocked,"not blocked");P();}

void t4(){T(rule_stored);
    AutoBlockRuleEngine eng;
    eng.addRule({"R1","stable","cert_fail",true});
    C(eng.ruleCount()==1,"ruleCount=1");P();}

void t5(){T(tier_match_required);
    AutoBlockRuleEngine eng;
    eng.addRule({"R1","stable","cert_fail",true});
    auto d=eng.evaluate("py->cpp","beta","cert_fail");
    C(!d.blocked,"not blocked diff tier");P();}

void t6(){T(trigger_match_required);
    AutoBlockRuleEngine eng;
    eng.addRule({"R1","stable","cert_fail",true});
    auto d=eng.evaluate("py->cpp","stable","gate_fail");
    C(!d.blocked,"not blocked diff trigger");P();}

void t7(){T(multiple_rules);
    AutoBlockRuleEngine eng;
    eng.addRule({"R1","stable","cert_fail",true});
    eng.addRule({"R2","beta","gate_fail",true});
    C(eng.ruleCount()==2,"2 rules");P();}

void t8(){T(ruleId_in_decision);
    AutoBlockRuleEngine eng;
    eng.addRule({"MYRULE","stable","cert_fail",true});
    auto d=eng.evaluate("py->cpp","stable","cert_fail");
    C(d.ruleId=="MYRULE","ruleId");P();}

int main(){
    std::cout<<"Step 844: AutoBlockRuleEngine\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
