// Step 780: Erlang lowering adapter v1 (10 tests)
#include "logic_actor/ErlangAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_erlang);auto x=ErlangAdapterV1::lower("spawn(fun()->ok end)");C(x.sourceLanguage=="erlang","lang");P();}
void t2(){T(non_empty_ir);auto x=ErlangAdapterV1::lower("spawn(fun()->ok end)");C(x.irSummary=="erlang_actor_ir_v1","ir");P();}
void t3(){T(empty_ir);auto x=ErlangAdapterV1::lower("");C(x.irSummary=="empty_erlang_unit","ir");P();}
void t4(){T(actor_detect_spawn);auto x=ErlangAdapterV1::lower("spawn(fun()->ok end)");C(x.actorModel,"actor");P();}
void t5(){T(actor_detect_receive);auto x=ErlangAdapterV1::lower("receive X -> ok end");C(x.actorModel,"actor");P();}
void t6(){T(supervision_detected);auto x=ErlangAdapterV1::lower("supervisor:start_link()");C(x.supervision,"sup");P();}
void t7(){T(supervision_false);auto x=ErlangAdapterV1::lower("spawn(fun()->ok end)");C(!x.supervision,"sup");P();}
void t8(){T(json_shape);auto j=ErlangAdapterV1::toJson(ErlangAdapterV1::lower("spawn(fun()->ok end)"));C(j.contains("source_language")&&j.contains("actor_model"),"shape");P();}
void t9(){T(machine_readable);auto j=ErlangAdapterV1::toJson(ErlangAdapterV1::lower("spawn(fun()->ok end)"));C(j.is_object(),"obj");P();}
void t10(){T(deterministic);auto a=ErlangAdapterV1::toJson(ErlangAdapterV1::lower("receive X -> ok end")).dump();auto b=ErlangAdapterV1::toJson(ErlangAdapterV1::lower("receive X -> ok end")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 780: ErlangAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
