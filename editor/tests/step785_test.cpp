// Step 785: Unmappable semantic blocklist + mandatory review gate (8 tests)
#include "logic_actor/SemanticBlocklistGate.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static LogicActorLoweringPacket mk(bool actor,bool bt){LogicActorLoweringPacket p; p.actorModel=actor; p.backtracking=bt; return p;}
void t1(){T(block_actor_to_c);auto x=SemanticBlocklistGate::evaluate(mk(true,false),"c");C(x.blocked,"blocked");P();}
void t2(){T(no_block_for_actor_to_erlang);auto x=SemanticBlocklistGate::evaluate(mk(true,false),"erlang");C(!x.blocked,"blocked");P();}
void t3(){T(reason_backtracking_loss);auto x=SemanticBlocklistGate::evaluate(mk(false,true),"cpp");bool ok=false;for(const auto&r:x.reasons)if(r=="backtracking_semantics_loss_risk")ok=true;C(ok,"reason");P();}
void t4(){T(review_required_when_blocked);auto x=SemanticBlocklistGate::evaluate(mk(true,false),"c");C(x.reviewRequired,"review");P();}
void t5(){T(review_required_when_reason_present);auto x=SemanticBlocklistGate::evaluate(mk(false,true),"cpp");C(x.reviewRequired,"review");P();}
void t6(){T(no_review_clean_case);auto x=SemanticBlocklistGate::evaluate(mk(false,false),"cpp");C(!x.reviewRequired,"review");P();}
void t7(){T(json_shape);auto j=SemanticBlocklistGate::toJson(SemanticBlocklistGate::evaluate(mk(true,false),"c"));C(j.contains("blocked")&&j.contains("reasons"),"shape");P();}
void t8(){T(deterministic);auto a=SemanticBlocklistGate::toJson(SemanticBlocklistGate::evaluate(mk(true,false),"c")).dump();auto b=SemanticBlocklistGate::toJson(SemanticBlocklistGate::evaluate(mk(true,false),"c")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 785: SemanticBlocklistGate\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
