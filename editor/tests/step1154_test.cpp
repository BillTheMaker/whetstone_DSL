#include "graduation/InteropCertificationPolicyBindings.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true); C(p0.policyId=="p1","i"); P();}
void t2(){T(min_rate); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true); C(p0.minPassRate==0.9,"m"); P();}
void t3(){T(oracle_required); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true); C(p0.requiresOracleMatch,"o"); P();}
void t4(){T(det_required); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true); C(p0.requiresDeterministicReplay,"d"); P();}
void t5(){T(active); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true); C(p0.bindingActive,"a"); P();}
void t6(){T(valid_true); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true); C(p0.valid,"v"); P();}
void t7(){T(valid_false_range); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",1.1,true,true,true); C(!p0.valid,"v"); P();}
void t8(){T(valid_false_flags); auto p0=InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,false,true); C(!p0.valid,"v"); P();}
void t9(){T(json_valid); auto j=InteropCertificationPolicyBindingsFactory::toJson(InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true)); C(j["valid"].get<bool>(),"j"); P();}
void t10(){T(deterministic); auto a=InteropCertificationPolicyBindingsFactory::toJson(InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true)); auto b=InteropCertificationPolicyBindingsFactory::toJson(InteropCertificationPolicyBindingsFactory::make("p1",0.9,true,true,true)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1154: Interop certification policy bindings\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
