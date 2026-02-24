#include "graduation/ReferenceImplementationHarnessForIREvidenceSpecs.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(id); auto h=ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true); C(h.harnessId=="h1","i"); P();}
void t2(){T(ir_spec); auto h=ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true); C(h.irSpecVersion=="ir-v1","s"); P();}
void t3(){T(ev_spec); auto h=ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true); C(h.evidenceSpecVersion=="ev-v1","s"); P();}
void t4(){T(languages); auto h=ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp","rust"},true); C(h.supportedLanguages.size()==2,"l"); P();}
void t5(){T(det_replay); auto h=ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true); C(h.deterministicReplay,"d"); P();}
void t6(){T(ready_true); auto h=ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true); C(h.ready,"r"); P();}
void t7(){T(ready_false); auto h=ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},false); C(!h.ready,"r"); P();}
void t8(){T(json_ready); auto j=ReferenceImplementationHarnessForIREvidenceSpecsFactory::toJson(ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true)); C(j["ready"].get<bool>(),"j"); P();}
void t9(){T(json_ir); auto j=ReferenceImplementationHarnessForIREvidenceSpecsFactory::toJson(ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true)); C(j["ir_spec_version"]=="ir-v1","j"); P();}
void t10(){T(deterministic); auto a=ReferenceImplementationHarnessForIREvidenceSpecsFactory::toJson(ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true)); auto b=ReferenceImplementationHarnessForIREvidenceSpecsFactory::toJson(ReferenceImplementationHarnessForIREvidenceSpecsFactory::make("h1","ir-v1","ev-v1",{"cpp"},true)); C(a.dump()==b.dump(),"d"); P();}
int main(){ std::cout<<"Step 1150: Reference implementation harness for IR/evidence specs\n"; t1();t2();t3();t4();t5();t6();t7();t8();t9();t10(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
