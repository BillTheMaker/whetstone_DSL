#include "graduation/ThirdPartyConformanceReplayRunner.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto r=ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9); C(r.runId=="run1","i"); P();}
void t2(){T(vendor); auto r=ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9); C(r.vendorId=="v1","v"); P();}
void t3(){T(scenarios); auto r=ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9); C(r.scenarioCount==10,"s"); P();}
void t4(){T(passed); auto r=ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9); C(r.passedCount==9,"p"); P();}
void t5(){T(rate); auto r=ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9); C(r.passRate==0.9,"r"); P();}
void t6(){T(conformant_true); auto r=ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9); C(r.conformant,"c"); P();}
void t7(){T(conformant_false); auto r=ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,8); C(!r.conformant,"c"); P();}
void t8(){T(json_rate); auto j=ThirdPartyConformanceReplayRunnerFactory::toJson(ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9)); C(j["pass_rate"]==0.9,"j"); P();}
void t9(){T(json_conformant); auto j=ThirdPartyConformanceReplayRunnerFactory::toJson(ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9)); C(j["conformant"].get<bool>(),"j"); P();}
void t10(){T(deterministic); auto a=ThirdPartyConformanceReplayRunnerFactory::toJson(ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9)); auto b=ThirdPartyConformanceReplayRunnerFactory::toJson(ThirdPartyConformanceReplayRunnerFactory::make("run1","v1",10,9)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1151: Third-party conformance replay runner\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
