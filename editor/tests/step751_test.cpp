// Step 751: TypeScript lowering adapter v2 (10 tests)
#include "dynamic/TypeScriptAdapterV2.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_ts);auto r=TypeScriptLoweringAdapterV2::lower("x");C(r.sourceLanguage=="typescript","lang");P();}
void t2(){T(non_empty_ir);auto r=TypeScriptLoweringAdapterV2::lower("x");C(r.irSummary=="ts_ir_v2","ir");P();}
void t3(){T(empty_ir);auto r=TypeScriptLoweringAdapterV2::lower("");C(r.irSummary=="empty_ts_unit","ir");P();}
void t4(){T(any_detected);auto r=TypeScriptLoweringAdapterV2::lower("let x:any = 1");C(r.dynamicDispatchCount==1,"any");P();}
void t5(){T(review_required_with_any);auto r=TypeScriptLoweringAdapterV2::lower("let x:any = 1");C(r.reviewRequired,"rev");P();}
void t6(){T(json_shape);auto j=TypeScriptLoweringAdapterV2::toJson(TypeScriptLoweringAdapterV2::lower("x"));C(j.contains("source_language")&&j.contains("ir_summary")&&j.contains("dynamic_dispatch_count")&&j.contains("review_required"),"shape");P();}
void t7(){T(json_lang);auto j=TypeScriptLoweringAdapterV2::toJson(TypeScriptLoweringAdapterV2::lower("x"));C(j.value("source_language","")=="typescript","lang");P();}
void t8(){T(machine_readable);auto j=TypeScriptLoweringAdapterV2::toJson(TypeScriptLoweringAdapterV2::lower("x"));C(j.is_object(),"obj");P();}
void t9(){T(deterministic);auto a=TypeScriptLoweringAdapterV2::toJson(TypeScriptLoweringAdapterV2::lower("let x:any = 1")).dump();auto b=TypeScriptLoweringAdapterV2::toJson(TypeScriptLoweringAdapterV2::lower("let x:any = 1")).dump();C(a==b,"det");P();}
void t10(){T(no_any_zero_dispatch);auto r=TypeScriptLoweringAdapterV2::lower("let x:number = 1");C(r.dynamicDispatchCount==0,"any");P();}
int main(){std::cout<<"Step 751: TypeScriptLoweringAdapterV2\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
