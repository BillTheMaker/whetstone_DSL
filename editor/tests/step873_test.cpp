// Step 873: Budget policy enforcer integration (10 tests)
#include "graduation/BudgetPolicyEnforcer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(within_budget_approved);
    BudgetPolicy pol{1.0f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.5f,pol);
    C(r.decision=="approved","approved");P();}
void t2(){T(over_budget_requires_token);
    BudgetPolicy pol{0.5f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.8f,pol);
    C(r.decision=="requires_token","token");P();}
void t3(){T(over_budget_with_token_approved);
    BudgetPolicy pol{0.5f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.8f,pol,true);
    C(r.decision=="approved","approved");P();}
void t4(){T(over_budget_no_require_rejected);
    BudgetPolicy pol{0.5f,false};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.8f,pol);
    C(r.decision=="rejected","rejected");P();}
void t5(){T(within_budget_flag);
    BudgetPolicy pol{1.0f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.5f,pol);
    C(r.withinBudget,"within");P();}
void t6(){T(over_budget_flag);
    BudgetPolicy pol{0.3f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.5f,pol);
    C(!r.withinBudget,"over");P();}
void t7(){T(pair_id_set);
    BudgetPolicy pol{1.0f,true};
    auto r=BudgetPolicyEnforcer::enforce("rust->go",0.3f,pol);
    C(r.pairId=="rust->go","pairId");P();}
void t8(){T(requested_cost_stored);
    BudgetPolicy pol{1.0f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.7f,pol);
    C(r.requestedCost==0.7f,"cost");P();}
void t9(){T(budget_limit_stored);
    BudgetPolicy pol{0.6f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.3f,pol);
    C(r.budgetLimit==0.6f,"limit");P();}
void t10(){T(to_json_has_decision);
    BudgetPolicy pol{1.0f,true};
    auto r=BudgetPolicyEnforcer::enforce("py->cpp",0.5f,pol);
    auto j=BudgetPolicyEnforcer::toJson(r);
    C(j.contains("decision"),"json");P();}

int main(){
    std::cout<<"Step 873: Budget policy enforcer\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
