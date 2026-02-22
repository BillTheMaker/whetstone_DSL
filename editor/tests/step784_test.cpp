// Step 784: Supervision tree preservation packet model (8 tests)
#include "logic_actor/SupervisionTreePacket.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static LogicActorLoweringPacket mk(bool sup){LogicActorLoweringPacket p; p.supervision=sup; return p;}
void t1(){T(supervision_detected_true);auto x=SupervisionTreePacketModel::build(mk(true),"erlang");C(x.supervisionDetected,"sup");P();}
void t2(){T(supervision_detected_false);auto x=SupervisionTreePacketModel::build(mk(false),"erlang");C(!x.supervisionDetected,"sup");P();}
void t3(){T(auto_preserve_erlang);auto x=SupervisionTreePacketModel::build(mk(true),"erlang");C(x.canPreserveAutomatically,"pres");P();}
void t4(){T(auto_preserve_elixir);auto x=SupervisionTreePacketModel::build(mk(true),"elixir");C(x.canPreserveAutomatically,"pres");P();}
void t5(){T(no_auto_preserve_cpp);auto x=SupervisionTreePacketModel::build(mk(true),"cpp");C(!x.canPreserveAutomatically,"pres");P();}
void t6(){T(confidence_high_when_preserved);auto x=SupervisionTreePacketModel::build(mk(true),"erlang");C(x.confidence==90,"conf");P();}
void t7(){T(confidence_low_when_not_preserved);auto x=SupervisionTreePacketModel::build(mk(true),"cpp");C(x.confidence==40,"conf");P();}
void t8(){T(deterministic);auto a=SupervisionTreePacketModel::toJson(SupervisionTreePacketModel::build(mk(true),"cpp")).dump();auto b=SupervisionTreePacketModel::toJson(SupervisionTreePacketModel::build(mk(true),"cpp")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 784: SupervisionTreePacket\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
