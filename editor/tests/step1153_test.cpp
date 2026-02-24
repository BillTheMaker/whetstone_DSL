#include "graduation/DivergenceTriageModelForInteropFailures.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto t=DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","medium","patch"); C(t.triageId=="t1","i"); P();}
void t2(){T(class_name); auto t=DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","medium","patch"); C(t.failureClass=="semantic","c"); P();}
void t3(){T(severity); auto t=DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","medium","patch"); C(t.severity=="medium","s"); P();}
void t4(){T(action); auto t=DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","medium","patch"); C(t.recommendedAction=="patch","a"); P();}
void t5(){T(escalation_true); auto t=DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","high","patch"); C(t.requiresEscalation,"e"); P();}
void t6(){T(escalation_false); auto t=DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","low","patch"); C(!t.requiresEscalation,"e"); P();}
void t7(){T(valid_true); auto t=DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","low","patch"); C(t.valid,"v"); P();}
void t8(){T(valid_false); auto t=DivergenceTriageModelForInteropFailuresFactory::make("","semantic","low","patch"); C(!t.valid,"v"); P();}
void t9(){T(json_valid); auto j=DivergenceTriageModelForInteropFailuresFactory::toJson(DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","low","patch")); C(j["valid"].get<bool>(),"j"); P();}
void t10(){T(deterministic); auto a=DivergenceTriageModelForInteropFailuresFactory::toJson(DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","low","patch")); auto b=DivergenceTriageModelForInteropFailuresFactory::toJson(DivergenceTriageModelForInteropFailuresFactory::make("t1","semantic","low","patch")); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1153: Divergence triage model for interop failures\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
