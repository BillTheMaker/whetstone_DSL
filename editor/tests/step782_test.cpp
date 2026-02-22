// Step 782: Logic-to-imperative projection policy set (10 tests)
#include "logic_actor/LogicImperativeProjectionPolicy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static LogicActorLoweringPacket mk(bool bt){LogicActorLoweringPacket p; p.backtracking=bt; return p;}
void t1(){T(policy_name_contains_target);auto x=LogicImperativeProjectionPolicy::choose(mk(false),"cpp");C(x.policy=="cpp_logic_projection","policy");P();}
void t2(){T(backtracking_preserved_for_prolog);auto x=LogicImperativeProjectionPolicy::choose(mk(true),"prolog");C(x.preservesBacktracking,"bt");P();}
void t3(){T(backtracking_not_preserved_for_cpp);auto x=LogicImperativeProjectionPolicy::choose(mk(true),"cpp");C(!x.preservesBacktracking,"bt");P();}
void t4(){T(review_required_on_loss);auto x=LogicImperativeProjectionPolicy::choose(mk(true),"cpp");C(x.reviewRequired,"review");P();}
void t5(){T(no_review_when_preserved);auto x=LogicImperativeProjectionPolicy::choose(mk(true),"prolog");C(!x.reviewRequired,"review");P();}
void t6(){T(no_review_when_no_backtracking);auto x=LogicImperativeProjectionPolicy::choose(mk(false),"cpp");C(!x.reviewRequired,"review");P();}
void t7(){T(json_shape);auto j=LogicImperativeProjectionPolicy::toJson(LogicImperativeProjectionPolicy::choose(mk(true),"cpp"));C(j.contains("policy")&&j.contains("review_required"),"shape");P();}
void t8(){T(machine_readable);auto j=LogicImperativeProjectionPolicy::toJson(LogicImperativeProjectionPolicy::choose(mk(true),"cpp"));C(j.is_object(),"obj");P();}
void t9(){T(deterministic);auto a=LogicImperativeProjectionPolicy::toJson(LogicImperativeProjectionPolicy::choose(mk(true),"cpp")).dump();auto b=LogicImperativeProjectionPolicy::toJson(LogicImperativeProjectionPolicy::choose(mk(true),"cpp")).dump();C(a==b,"det");P();}
void t10(){T(policy_for_java);auto x=LogicImperativeProjectionPolicy::choose(mk(false),"java");C(x.policy=="java_logic_projection","policy");P();}
int main(){std::cout<<"Step 782: LogicImperativeProjectionPolicy\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
