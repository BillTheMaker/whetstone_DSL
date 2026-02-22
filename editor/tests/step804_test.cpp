// Step 804: Memory layout checker (10 tests)
#include "low_level/MemoryLayoutChecker.h"
#include "low_level/AbiCallingConventionModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static AbiPacket mk(std::string layout){AbiPacket p; p.memoryLayout=layout; return p;}
void t1(){T(compatible_same);auto x=MemoryLayoutChecker::assess(mk("default"),mk("default"));C(x.compatible,"comp");P();}
void t2(){T(incompatible_diff);auto x=MemoryLayoutChecker::assess(mk("default"),mk("packed"));C(!x.compatible,"comp");P();}
void t3(){T(reason_set);auto x=MemoryLayoutChecker::assess(mk("default"),mk("packed"));C(x.reason=="layout_mismatch","reason");P();}
void t4(){T(json_shape);auto j=MemoryLayoutChecker::toJson(MemoryLayoutChecker::assess(mk("default"),mk("packed")));C(j.contains("layout")&&j.contains("compatible"),"shape");P();}
void t5(){T(machine_readable);auto j=MemoryLayoutChecker::toJson(MemoryLayoutChecker::assess(mk("default"),mk("packed")));C(j.is_object(),"obj");P();}
void t6(){T(deterministic);auto a=MemoryLayoutChecker::toJson(MemoryLayoutChecker::assess(mk("default"),mk("packed"))).dump();auto b=MemoryLayoutChecker::toJson(MemoryLayoutChecker::assess(mk("default"),mk("packed"))).dump();C(a==b,"det");P();}
void t7(){T(layout_string);auto x=MemoryLayoutChecker::assess(mk("foo"),mk("foo"));C(x.layout=="foo/foo","layout");P();}
void t8(){T(non_matching_reason);auto x=MemoryLayoutChecker::assess(mk("foo"),mk("bar"));C(x.reason=="layout_mismatch","reason");P();}
void t9(){T(compatible_false);auto x=MemoryLayoutChecker::assess(mk("foo"),mk("bar"));C(!x.compatible,"comp");P();}
void t10(){T(compatible_true);auto x=MemoryLayoutChecker::assess(mk("foo"),mk("foo"));C(x.compatible,"comp");P();}
int main(){std::cout<<"Step 804: MemoryLayoutChecker\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
