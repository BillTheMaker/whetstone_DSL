// Step 753: Lua lowering adapter v1 (10 tests)
#include "dynamic/LuaAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_lua);auto r=LuaLoweringAdapterV1::lower("x");C(r.sourceLanguage=="lua","lang");P();}
void t2(){T(non_empty_ir);auto r=LuaLoweringAdapterV1::lower("x");C(r.irSummary=="lua_ir_v1","ir");P();}
void t3(){T(empty_ir);auto r=LuaLoweringAdapterV1::lower("");C(r.irSummary=="empty_lua_unit","ir");P();}
void t4(){T(setmetatable_detected);auto r=LuaLoweringAdapterV1::lower("setmetatable(t,{})");C(r.dynamicDispatchCount==1,"disp");P();}
void t5(){T(review_required);auto r=LuaLoweringAdapterV1::lower("setmetatable(t,{})");C(r.reviewRequired,"rev");P();}
void t6(){T(json_shape);auto j=LuaLoweringAdapterV1::toJson(LuaLoweringAdapterV1::lower("x"));C(j.contains("source_language")&&j.contains("ir_summary")&&j.contains("dynamic_dispatch_count")&&j.contains("review_required"),"shape");P();}
void t7(){T(json_lang);auto j=LuaLoweringAdapterV1::toJson(LuaLoweringAdapterV1::lower("x"));C(j.value("source_language","")=="lua","lang");P();}
void t8(){T(machine_readable);auto j=LuaLoweringAdapterV1::toJson(LuaLoweringAdapterV1::lower("x"));C(j.is_object(),"obj");P();}
void t9(){T(deterministic);auto a=LuaLoweringAdapterV1::toJson(LuaLoweringAdapterV1::lower("setmetatable(t,{})")).dump();auto b=LuaLoweringAdapterV1::toJson(LuaLoweringAdapterV1::lower("setmetatable(t,{})")).dump();C(a==b,"det");P();}
void t10(){T(no_pattern_zero_dispatch);auto r=LuaLoweringAdapterV1::lower("print(x)");C(r.dynamicDispatchCount==0,"disp");P();}
int main(){std::cout<<"Step 753: LuaLoweringAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
