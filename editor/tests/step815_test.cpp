// Step 815: Migration readiness scoring (8 tests)
#include "legacy_ingestion/MigrationReadiness.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(ready_true);auto s=MigrationReadiness::evaluate(0.7,6);C(s.ready,"ready");P();}
void t2(){T(ready_false_low);auto s=MigrationReadiness::evaluate(0.4,1);C(!s.ready,"ready");P();}
void t3(){T(completeness_caps);auto s=MigrationReadiness::evaluate(0.9,10);C(s.completeness==1.0,"complete");P();}
void t4(){T(confidence_return);auto s=MigrationReadiness::evaluate(0.6,3);C(s.confidence==0.6,"conf");P();}
void t5(){T(json_shape);auto j=MigrationReadiness::toJson(MigrationReadiness::evaluate(0.6,3));C(j.contains("ready"),"shape");P();}
void t6(){T(machine_readable);auto j=MigrationReadiness::toJson(MigrationReadiness::evaluate(0.6,3));C(j.is_object(),"obj");P();}
void t7(){T(deterministic);auto a=MigrationReadiness::toJson(MigrationReadiness::evaluate(0.6,3)).dump();auto b=MigrationReadiness::toJson(MigrationReadiness::evaluate(0.6,3)).dump();C(a==b,"det");P();}
void t8(){T(completeness_fraction);auto s=MigrationReadiness::evaluate(0.9,2);C(s.completeness==0.4,"complete");P();}
int main(){std::cout<<"Step 815: MigrationReadiness\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
