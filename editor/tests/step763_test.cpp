// Step 763: Nullability + optionality canonical model bridge (10 tests)
#include "managed/NullabilityOptionalityBridge.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static ManagedLoweringPacket mk(bool nullable,bool optional){ManagedLoweringPacket x; x.hasNullableSyntax=nullable; x.hasOptionalType=optional; return x;}
void t1(){T(option_priority);auto x=NullabilityOptionalityBridge::fromLowering(mk(true,true));C(x.canonicalModel=="option","model");P();}
void t2(){T(nullable_model);auto x=NullabilityOptionalityBridge::fromLowering(mk(true,false));C(x.canonicalModel=="nullable","model");P();}
void t3(){T(nonnullable_model);auto x=NullabilityOptionalityBridge::fromLowering(mk(false,false));C(x.canonicalModel=="nonnullable","model");P();}
void t4(){T(nullable_flag);auto x=NullabilityOptionalityBridge::fromLowering(mk(true,false));C(x.nullableDetected,"nullable");P();}
void t5(){T(optional_flag);auto x=NullabilityOptionalityBridge::fromLowering(mk(false,true));C(x.optionalDetected,"optional");P();}
void t6(){T(note_non_empty);auto x=NullabilityOptionalityBridge::fromLowering(mk(true,false));C(!x.bridgeNote.empty(),"note");P();}
void t7(){T(json_shape);auto j=NullabilityOptionalityBridge::toJson(NullabilityOptionalityBridge::fromLowering(mk(false,true)));C(j.contains("canonical_model")&&j.contains("bridge_note"),"shape");P();}
void t8(){T(machine_readable);auto j=NullabilityOptionalityBridge::toJson(NullabilityOptionalityBridge::fromLowering(mk(false,true)));C(j.is_object(),"obj");P();}
void t9(){T(deterministic);auto a=NullabilityOptionalityBridge::toJson(NullabilityOptionalityBridge::fromLowering(mk(true,false))).dump();auto b=NullabilityOptionalityBridge::toJson(NullabilityOptionalityBridge::fromLowering(mk(true,false))).dump();C(a==b,"det");P();}
void t10(){T(bridge_note_mentions_nullable);auto x=NullabilityOptionalityBridge::fromLowering(mk(true,false));C(x.bridgeNote.find("nullable")!=std::string::npos,"note");P();}
int main(){std::cout<<"Step 763: NullabilityOptionalityBridge\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
