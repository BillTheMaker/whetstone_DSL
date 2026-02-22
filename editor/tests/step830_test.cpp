// Step 830: TierPromotionEngine tests (10 tests)
#include "graduation/TierPromotionEngine.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(beta_stable_promotion_success);
    PromotionRequest req{"python->cpp","beta","stable",100,100};
    auto r=TierPromotionEngine::evaluate(req);
    C(r.promoted,"promoted");P();}

void t2(){T(promotion_fails_insufficient_tests);
    PromotionRequest req{"python->cpp","beta","stable",50,100};
    auto r=TierPromotionEngine::evaluate(req);
    C(!r.promoted,"not promoted");P();}

void t3(){T(experimental_beta_promotion);
    PromotionRequest req{"rust->go","experimental","beta",10,10};
    auto r=TierPromotionEngine::evaluate(req);
    C(r.promoted,"promoted");P();}

void t4(){T(invalid_tier_transition);
    PromotionRequest req{"cpp->js","stable","experimental",10,10};
    auto r=TierPromotionEngine::evaluate(req);
    C(!r.promoted,"not promoted");
    C(r.reason=="invalid_tier_transition","reason");P();}

void t5(){T(empty_pairId_error);
    std::string err;
    PromotionRequest req{"","beta","stable",10,10};
    TierPromotionEngine::evaluate(req,&err);
    C(err=="pair_id_missing","error");P();}

void t6(){T(newTier_equals_targetTier_when_promoted);
    PromotionRequest req{"py->cpp","beta","stable",5,5};
    auto r=TierPromotionEngine::evaluate(req);
    C(r.newTier=="stable","newTier");P();}

void t7(){T(reason_criteria_met_on_success);
    PromotionRequest req{"py->cpp","beta","stable",5,5};
    auto r=TierPromotionEngine::evaluate(req);
    C(r.reason=="criteria_met","reason");P();}

void t8(){T(reason_invalid_tier_transition);
    PromotionRequest req{"py->cpp","stable","experimental",5,5};
    auto r=TierPromotionEngine::evaluate(req);
    C(r.reason=="invalid_tier_transition","reason");P();}

void t9(){T(required_tests_zero_fails);
    PromotionRequest req{"py->cpp","beta","stable",5,0};
    auto r=TierPromotionEngine::evaluate(req);
    C(!r.promoted,"not promoted when required=0");P();}

void t10(){T(passing_tests_gte_required);
    PromotionRequest req{"py->cpp","experimental","beta",10,8};
    auto r=TierPromotionEngine::evaluate(req);
    C(r.promoted,"promoted when passing>=required");P();}

int main(){
    std::cout<<"Step 830: TierPromotionEngine\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
