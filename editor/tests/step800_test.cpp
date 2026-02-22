// Step 800: C interop adapter deepening (10 tests)
#include "low_level/CInteropAdapter.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_c);auto x=CInteropAdapter::describe("fun");C(x.language=="c","lang");P();}
void t2(){T(stdcall_override);auto x=CInteropAdapter::describe("__stdcall fun");C(x.callingConvention=="stdcall","conv");P();}
void t3(){T(memory_default);auto x=CInteropAdapter::describe("fun");C(x.memoryLayout=="default","layout");P();}
void t4(){T(host_false);auto x=CInteropAdapter::describe("fun");C(!x.hostBoundary,"host");P();}
void t5(){T(host_true);auto x=CInteropAdapter::describe("import fun");C(x.hostBoundary,"host");P();}
void t6(){T(json_shape);auto j=CInteropAdapter::toJson(CInteropAdapter::describe(""));C(j.contains("language"),"shape");P();}
void t7(){T(machine_readable);auto j=CInteropAdapter::toJson(CInteropAdapter::describe(""));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=CInteropAdapter::toJson(CInteropAdapter::describe(""));auto b=CInteropAdapter::toJson(CInteropAdapter::describe(""));C(a==b,"det");P();}
void t9(){T(callconv_independence);auto x=CInteropAdapter::describe("cdecl");C(x.callingConvention=="cdecl","conv");P();}
void t10(){T(host_mention);auto x=CInteropAdapter::describe("export fun");C(x.hostBoundary,"host");P();}
int main(){std::cout<<"Step 800: CInteropAdapter\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
