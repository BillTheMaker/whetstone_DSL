// Step 730: Sanitizer gate integration (10 tests)
#include "gates/SanitizerGateIntegration.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(default_requires_both);SanitizerGateProfile profile;C(profile.requireAsan&&profile.requireUbsan,"req");P();}
void t2(){T(pass_when_both_clean);auto r=SanitizerGateIntegration::evaluate(SanitizerGateProfile{},true,true);C(r.pass,"pass");P();}
void t3(){T(fail_when_asan_dirty);auto r=SanitizerGateIntegration::evaluate(SanitizerGateProfile{},false,true);C(!r.pass,"fail");P();}
void t4(){T(fail_when_ubsan_dirty);auto r=SanitizerGateIntegration::evaluate(SanitizerGateProfile{},true,false);C(!r.pass,"fail");P();}
void t5(){T(optional_asan_respected);SanitizerGateProfile profile; profile.requireAsan=false;auto r=SanitizerGateIntegration::evaluate(profile,false,true);C(r.pass,"opt");P();}
void t6(){T(optional_ubsan_respected);SanitizerGateProfile profile; profile.requireUbsan=false;auto r=SanitizerGateIntegration::evaluate(profile,true,false);C(r.pass,"opt");P();}
void t7(){T(json_shape);auto j=SanitizerGateIntegration::toJson(SanitizerGateIntegration::evaluate(SanitizerGateProfile{},true,true));C(j.contains("asan_clean")&&j.contains("ubsan_clean")&&j.contains("pass"),"shape");P();}
void t8(){T(machine_readable);auto j=SanitizerGateIntegration::toJson(SanitizerGateIntegration::evaluate(SanitizerGateProfile{},true,true));C(j.is_object(),"obj");P();}
void t9(){T(clean_flags_preserved);auto r=SanitizerGateIntegration::evaluate(SanitizerGateProfile{},true,true);C(r.asanClean&&r.ubsanClean,"flags");P();}
void t10(){T(deterministic);auto a=SanitizerGateIntegration::toJson(SanitizerGateIntegration::evaluate(SanitizerGateProfile{},true,true)).dump();auto b=SanitizerGateIntegration::toJson(SanitizerGateIntegration::evaluate(SanitizerGateProfile{},true,true)).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 730: SanitizerGateIntegration\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
