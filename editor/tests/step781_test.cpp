// Step 781: Elixir lowering adapter v1 (10 tests)
#include "logic_actor/ElixirAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_elixir);auto x=ElixirAdapterV1::lower("spawn(fn -> :ok end)");C(x.sourceLanguage=="elixir","lang");P();}
void t2(){T(non_empty_ir);auto x=ElixirAdapterV1::lower("spawn(fn -> :ok end)");C(x.irSummary=="elixir_actor_ir_v1","ir");P();}
void t3(){T(empty_ir);auto x=ElixirAdapterV1::lower("");C(x.irSummary=="empty_elixir_unit","ir");P();}
void t4(){T(actor_detect_spawn);auto x=ElixirAdapterV1::lower("spawn(fn -> :ok end)");C(x.actorModel,"actor");P();}
void t5(){T(actor_detect_receive);auto x=ElixirAdapterV1::lower("receive do x -> x end");C(x.actorModel,"actor");P();}
void t6(){T(supervision_detected);auto x=ElixirAdapterV1::lower("Supervisor.start_link([])");C(x.supervision,"sup");P();}
void t7(){T(macro_surface_detected);auto x=ElixirAdapterV1::lower("defmacro m do :ok end");C(x.macroSurface,"macro");P();}
void t8(){T(json_shape);auto j=ElixirAdapterV1::toJson(ElixirAdapterV1::lower("spawn(fn -> :ok end)"));C(j.contains("source_language")&&j.contains("macro_surface"),"shape");P();}
void t9(){T(machine_readable);auto j=ElixirAdapterV1::toJson(ElixirAdapterV1::lower("spawn(fn -> :ok end)"));C(j.is_object(),"obj");P();}
void t10(){T(deterministic);auto a=ElixirAdapterV1::toJson(ElixirAdapterV1::lower("defmacro m do :ok end")).dump();auto b=ElixirAdapterV1::toJson(ElixirAdapterV1::lower("defmacro m do :ok end")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 781: ElixirAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
