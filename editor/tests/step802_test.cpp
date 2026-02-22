// Step 802: x86 assembly adapter v1 (10 tests)
#include "low_level/X86AdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_x86);auto x=X86AdapterV1::describe("mov");C(x.language=="x86","lang");P();}
void t2(){T(fastcall_detected);auto x=X86AdapterV1::describe("fastcall func");C(x.callingConvention=="fastcall","conv");P();}
void t3(){T(host_boundary_true);auto x=X86AdapterV1::describe("orphan");C(x.hostBoundary,"host");P();}
void t4(){T(host_boundary_false);auto x=X86AdapterV1::describe("mov");C(!x.hostBoundary,"host");P();}
void t5(){T(memory_default);auto x=X86AdapterV1::describe("mov");C(x.memoryLayout=="default","layout");P();}
void t6(){T(json_shape);auto j=X86AdapterV1::toJson(X86AdapterV1::describe(""));C(j.contains("language"),"shape");P();}
void t7(){T(machine_readable);auto j=X86AdapterV1::toJson(X86AdapterV1::describe(""));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=X86AdapterV1::toJson(X86AdapterV1::describe(""));auto b=X86AdapterV1::toJson(X86AdapterV1::describe(""));C(a==b,"det");P();}
void t9(){T(no_fastcall_default);auto x=X86AdapterV1::describe("mov");C(x.callingConvention=="cdecl","conv");P();}
void t10(){T(host_mention);auto x=X86AdapterV1::describe("orphan" );C(x.hostBoundary,"host");P();}
int main(){std::cout<<"Step 802: X86AdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
