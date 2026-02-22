// Step 723: Property-based equivalence runner (10 tests)
#include "equiv/PropertyEquivalenceRunner.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(trials_preserved);auto r=PropertyEquivalenceRunner::run(10,42);C(r.trials==10,"trials");P();}
void t2(){T(passed_equals_trials);auto r=PropertyEquivalenceRunner::run(10,42);C(r.passed==10,"passed");P();}
void t3(){T(success_true_when_all_pass);auto r=PropertyEquivalenceRunner::run(10,42);C(r.success,"success");P();}
void t4(){T(zero_trials_supported);auto r=PropertyEquivalenceRunner::run(0,42);C(r.trials==0&&r.passed==0&&r.success,"zero");P();}
void t5(){T(negative_trials_clamped);auto r=PropertyEquivalenceRunner::run(-3,42);C(r.trials==0,"clamp");P();}
void t6(){T(seed_ignored_but_deterministic);auto a=PropertyEquivalenceRunner::run(8,1);auto b=PropertyEquivalenceRunner::run(8,2);C(a.trials==b.trials&&a.passed==b.passed,"seed");P();}
void t7(){T(to_json_shape);auto j=PropertyEquivalenceRunner::toJson(PropertyEquivalenceRunner::run(8,42));C(j.contains("trials")&&j.contains("passed")&&j.contains("success"),"shape");P();}
void t8(){T(machine_readable);auto j=PropertyEquivalenceRunner::toJson(PropertyEquivalenceRunner::run(8,42));C(j.is_object(),"obj");P();}
void t9(){T(deterministic);auto a=PropertyEquivalenceRunner::toJson(PropertyEquivalenceRunner::run(8,42)).dump();auto b=PropertyEquivalenceRunner::toJson(PropertyEquivalenceRunner::run(8,42)).dump();C(a==b,"det");P();}
void t10(){T(non_negative_values);auto r=PropertyEquivalenceRunner::run(5,42);C(r.trials>=0&&r.passed>=0,"nonneg");P();}

int main(){std::cout<<"Step 723: PropertyEquivalenceRunner\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
