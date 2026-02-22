// Step 779: Prolog lowering adapter v1 (12 tests)
#include "logic_actor/PrologAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_prolog);auto x=PrologAdapterV1::lower("p(X).");C(x.sourceLanguage=="prolog","lang");P();}
void t2(){T(non_empty_ir);auto x=PrologAdapterV1::lower("p(X).");C(x.irSummary=="prolog_logic_ir_v1","ir");P();}
void t3(){T(empty_ir);auto x=PrologAdapterV1::lower("");C(x.irSummary=="empty_prolog_unit","ir");P();}
void t4(){T(query_arity_detected);auto x=PrologAdapterV1::lower("p(X).");C(x.queryArity==1,"arity");P();}
void t5(){T(query_arity_zero);auto x=PrologAdapterV1::lower("fact.");C(x.queryArity==0,"arity");P();}
void t6(){T(backtracking_with_semicolon);auto x=PrologAdapterV1::lower("p(X); q(X).");C(x.backtracking,"bt");P();}
void t7(){T(backtracking_with_fail);auto x=PrologAdapterV1::lower("p(X), fail.");C(x.backtracking,"bt");P();}
void t8(){T(no_backtracking_simple_fact);auto x=PrologAdapterV1::lower("p(1).");C(!x.backtracking,"bt");P();}
void t9(){T(json_shape);auto j=PrologAdapterV1::toJson(PrologAdapterV1::lower("p(X)."));C(j.contains("source_language")&&j.contains("backtracking"),"shape");P();}
void t10(){T(machine_readable);auto j=PrologAdapterV1::toJson(PrologAdapterV1::lower("p(X)."));C(j.is_object(),"obj");P();}
void t11(){T(deterministic);auto a=PrologAdapterV1::toJson(PrologAdapterV1::lower("p(X); q(X).")).dump();auto b=PrologAdapterV1::toJson(PrologAdapterV1::lower("p(X); q(X).")).dump();C(a==b,"det");P();}
void t12(){T(backtracking_json_true);auto j=PrologAdapterV1::toJson(PrologAdapterV1::lower("p(X); q(X)."));C(j.value("backtracking",false),"json");P();}
int main(){std::cout<<"Step 779: PrologAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
