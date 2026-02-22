// Step 801: WASM adapter v2 (10 tests)
#include "low_level/WasmAdapterV2.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_wasm);auto x=WasmAdapterV2::describe("module");C(x.language=="wasm","lang");P();}
void t2(){T(calling_model);auto x=WasmAdapterV2::describe("module");C(x.callingConvention=="wasmabi","conv");P();}
void t3(){T(memory_linear);auto x=WasmAdapterV2::describe("module");C(x.memoryLayout=="linear","layout");P();}
void t4(){T(host_boundary_false);auto x=WasmAdapterV2::describe("module");C(!x.hostBoundary,"host");P();}
void t5(){T(host_boundary_true);auto x=WasmAdapterV2::describe("host import");C(x.hostBoundary,"host");P();}
void t6(){T(json_shape);auto j=WasmAdapterV2::toJson(WasmAdapterV2::describe(""));C(j.contains("language")&&j.contains("calling_convention"),"shape");P();}
void t7(){T(machine_readable);auto j=WasmAdapterV2::toJson(WasmAdapterV2::describe(""));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=WasmAdapterV2::toJson(WasmAdapterV2::describe(""));auto b=WasmAdapterV2::toJson(WasmAdapterV2::describe(""));C(a==b,"det");P();}
void t9(){T(host_keyword);auto x=WasmAdapterV2::describe("import host");C(x.hostBoundary,"host");P();}
void t10(){T(deterministic_again);auto a=WasmAdapterV2::toJson(WasmAdapterV2::describe("host"));auto b=WasmAdapterV2::toJson(WasmAdapterV2::describe("host"));C(a==b,"det");P();}
int main(){std::cout<<"Step 801: WasmAdapterV2\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
