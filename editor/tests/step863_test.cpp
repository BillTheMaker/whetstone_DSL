// Step 863: A/B harness for hint effectiveness (10 tests)
#include "graduation/HintABHarness.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(experiment_id_set);
    ABVariant ctrl{"control",10,5}; ABVariant trt{"hint_guided",10,8};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(r.experimentId=="exp-1","id");P();}
void t2(){T(lift_positive_when_treatment_better);
    ABVariant ctrl{"control",10,5}; ABVariant trt{"hint_guided",10,8};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(r.lift>0.0f,"lift");P();}
void t3(){T(treatment_wins_when_lift_gt_threshold);
    ABVariant ctrl{"control",10,4}; ABVariant trt{"hint_guided",10,9};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(r.treatmentWins,"wins");P();}
void t4(){T(treatment_not_wins_small_lift);
    ABVariant ctrl{"control",10,5}; ABVariant trt{"hint_guided",10,5};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(!r.treatmentWins,"no win");P();}
void t5(){T(zero_total_zero_rate);
    ABVariant ctrl{"control",0,0}; ABVariant trt{"hint_guided",0,0};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(r.lift==0.0f,"zero");P();}
void t6(){T(control_stored);
    ABVariant ctrl{"control",10,5}; ABVariant trt{"hint_guided",10,8};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(r.control.variantId=="control","ctrl");P();}
void t7(){T(treatment_stored);
    ABVariant ctrl{"control",10,5}; ABVariant trt{"hint_guided",10,8};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(r.treatment.variantId=="hint_guided","trt");P();}
void t8(){T(negative_lift_treatment_loses);
    ABVariant ctrl{"control",10,8}; ABVariant trt{"hint_guided",10,4};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    C(!r.treatmentWins,"lose");P();}
void t9(){T(to_json_has_lift);
    ABVariant ctrl{"control",10,5}; ABVariant trt{"hint_guided",10,8};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    auto j=HintABHarness::toJson(r);
    C(j.contains("lift"),"lift");P();}
void t10(){T(to_json_treatment_wins);
    ABVariant ctrl{"control",10,4}; ABVariant trt{"hint_guided",10,9};
    auto r=HintABHarness::evaluate("exp-1",ctrl,trt);
    auto j=HintABHarness::toJson(r);
    C(j.contains("treatment_wins"),"wins");P();}

int main(){
    std::cout<<"Step 863: A/B harness for hint effectiveness\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
