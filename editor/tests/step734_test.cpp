// Step 734: Gate severity policy (8 tests)
#include "gates/GateSeverityPolicy.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(security_block);C(GateSeverityPolicy::severityFor("security")=="block","sec");P();}
void t2(){T(sanitizer_block);C(GateSeverityPolicy::severityFor("sanitizer")=="block","san");P();}
void t3(){T(performance_warn);C(GateSeverityPolicy::severityFor("performance")=="warn","perf");P();}
void t4(){T(supply_chain_warn);C(GateSeverityPolicy::severityFor("supply_chain")=="warn","sup");P();}
void t5(){T(default_info);C(GateSeverityPolicy::severityFor("other")=="info","info");P();}
void t6(){T(to_json_shape);auto j=GateSeverityPolicy::toJson("security");C(j.contains("gate")&&j.contains("severity"),"shape");P();}
void t7(){T(machine_readable);auto j=GateSeverityPolicy::toJson("security");C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=GateSeverityPolicy::toJson("performance").dump();auto b=GateSeverityPolicy::toJson("performance").dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 734: GateSeverityPolicy\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
