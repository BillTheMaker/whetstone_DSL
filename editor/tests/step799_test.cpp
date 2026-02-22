// Step 799: ABI/calling-convention model (12 tests)
#include "low_level/AbiCallingConventionModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(language_set);auto x=AbiCallingConventionModel::describe("","c");C(x.language=="c","lang");P();}
void t2(){T(default_cconv);auto x=AbiCallingConventionModel::describe("","c");C(x.callingConvention=="cdecl","conv");P();}
void t3(){T(stdcall_detected);auto x=AbiCallingConventionModel::describe("__stdcall func","c");C(x.callingConvention=="stdcall","conv");P();}
void t4(){T(memory_layout_default);auto x=AbiCallingConventionModel::describe("","c");C(x.memoryLayout=="default","layout");P();}
void t5(){T(memory_layout_packed);auto x=AbiCallingConventionModel::describe("packed struct","c");C(x.memoryLayout=="packed","layout");P();}
void t6(){T(host_boundary_false);auto x=AbiCallingConventionModel::describe("","c");C(!x.hostBoundary,"host");P();}
void t7(){T(host_boundary_true);auto x=AbiCallingConventionModel::describe("import func","c");C(x.hostBoundary,"host");P();}
void t8(){T(json_shape);auto j=AbiCallingConventionModel::toJson(AbiCallingConventionModel::describe("import","wasm"));C(j.contains("language")&&j.contains("calling_convention"),"shape");P();}
void t9(){T(machine_readable);auto j=AbiCallingConventionModel::toJson(AbiCallingConventionModel::describe("","wasm"));C(j.is_object(),"obj");P();}
void t10(){T(deterministic);auto a=AbiCallingConventionModel::toJson(AbiCallingConventionModel::describe("","c")).dump();auto b=AbiCallingConventionModel::toJson(AbiCallingConventionModel::describe("","c")).dump();C(a==b,"det");P();}
void t11(){T(override_conv);auto x=AbiCallingConventionModel::describe("stdcall","c");C(x.callingConvention=="stdcall","conv");P();}
void t12(){T(host_string);auto x=AbiCallingConventionModel::describe("export","wasm");C(x.hostBoundary,"host");P();}
int main(){std::cout<<"Step 799: AbiCallingConventionModel\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
