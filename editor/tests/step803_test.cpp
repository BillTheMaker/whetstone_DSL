// Step 803: ARM assembly adapter v1 (10 tests)
#include "low_level/ARMAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_arm);auto x=ARMAdapterV1::describe("mov");C(x.language=="arm","lang");P();}
void t2(){T(aapcs_detected);auto x=ARMAdapterV1::describe("aapcs fun");C(x.callingConvention=="aapcs","conv");P();}
void t3(){T(memory_packed);auto x=ARMAdapterV1::describe("packed struct");C(x.memoryLayout=="packed","layout");P();}
void t4(){T(memory_default);auto x=ARMAdapterV1::describe("standard");C(x.memoryLayout=="arm_default","layout");P();}
void t5(){T(json_shape);auto j=ARMAdapterV1::toJson(ARMAdapterV1::describe(""));C(j.contains("language")&&j.contains("memory_layout"),"shape");P();}
void t6(){T(machine_readable);auto j=ARMAdapterV1::toJson(ARMAdapterV1::describe(""));C(j.is_object(),"obj");P();}
void t7(){T(deterministic);auto a=ARMAdapterV1::toJson(ARMAdapterV1::describe(""));auto b=ARMAdapterV1::toJson(ARMAdapterV1::describe(""));C(a==b,"det");P();}
void t8(){T(no_aapcs_default);auto x=ARMAdapterV1::describe("" );C(x.callingConvention=="cdecl","conv");P();}
void t9(){T(host_boundary_false);auto x=ARMAdapterV1::describe("" );C(!x.hostBoundary,"host");P();}
void t10(){T(host_boundary_true);auto x=ARMAdapterV1::describe("import" );C(x.hostBoundary,"host");P();}
int main(){std::cout<<"Step 803: ARMAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
