// Step 783: Actor-to-thread/async projection policy set (10 tests)
#include "logic_actor/ActorAsyncProjectionPolicy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static LogicActorLoweringPacket mk(bool actor){LogicActorLoweringPacket p; p.actorModel=actor; return p;}
void t1(){T(policy_name_contains_target);auto x=ActorAsyncProjectionPolicy::choose(mk(true),"cpp");C(x.policy=="cpp_actor_projection","policy");P();}
void t2(){T(mailbox_modeled_erlang);auto x=ActorAsyncProjectionPolicy::choose(mk(true),"erlang");C(x.mailboxModeled,"mailbox");P();}
void t3(){T(mailbox_modeled_elixir);auto x=ActorAsyncProjectionPolicy::choose(mk(true),"elixir");C(x.mailboxModeled,"mailbox");P();}
void t4(){T(mailbox_not_modeled_cpp);auto x=ActorAsyncProjectionPolicy::choose(mk(true),"cpp");C(!x.mailboxModeled,"mailbox");P();}
void t5(){T(review_required_on_actor_loss);auto x=ActorAsyncProjectionPolicy::choose(mk(true),"cpp");C(x.reviewRequired,"review");P();}
void t6(){T(no_review_with_erlang);auto x=ActorAsyncProjectionPolicy::choose(mk(true),"erlang");C(!x.reviewRequired,"review");P();}
void t7(){T(no_review_without_actor);auto x=ActorAsyncProjectionPolicy::choose(mk(false),"cpp");C(!x.reviewRequired,"review");P();}
void t8(){T(json_shape);auto j=ActorAsyncProjectionPolicy::toJson(ActorAsyncProjectionPolicy::choose(mk(true),"cpp"));C(j.contains("policy")&&j.contains("mailbox_modeled"),"shape");P();}
void t9(){T(machine_readable);auto j=ActorAsyncProjectionPolicy::toJson(ActorAsyncProjectionPolicy::choose(mk(true),"cpp"));C(j.is_object(),"obj");P();}
void t10(){T(deterministic);auto a=ActorAsyncProjectionPolicy::toJson(ActorAsyncProjectionPolicy::choose(mk(true),"cpp")).dump();auto b=ActorAsyncProjectionPolicy::toJson(ActorAsyncProjectionPolicy::choose(mk(true),"cpp")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 783: ActorAsyncProjectionPolicy\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
