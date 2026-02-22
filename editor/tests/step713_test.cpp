// Step 713: error model mapping (10 tests)

#include "cpp_ir/CppErrorModelMapping.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static SemanticCoreIR sample(){SemanticCoreIR ir;ir.moduleId="m";ir.contracts={{"errorBehavior",nlohmann::json::array({"panic_possible"})}};return ir;}

void t1(){T(profile_preserved);auto d=CppErrorModelMapping::map(sample(),"safe-first");C(d.profile=="safe-first","profile");P();}
void t2(){T(safe_first_expected);auto d=CppErrorModelMapping::map(sample(),"safe-first");C(d.resultType=="expected","resultType");P();}
void t3(){T(perf_first_expected);auto d=CppErrorModelMapping::map(sample(),"perf-first");C(d.resultType=="expected","resultType");P();}
void t4(){T(interop_first_status_or);auto d=CppErrorModelMapping::map(sample(),"interop-first");C(d.resultType=="status_or","resultType");P();}
void t5(){T(panic_to_terminate_true);auto d=CppErrorModelMapping::map(sample());C(d.panicTranslatedToTerminate,"panic flag");P();}
void t6(){T(no_panic_false);SemanticCoreIR ir;ir.moduleId="m";auto d=CppErrorModelMapping::map(ir);C(!d.panicTranslatedToTerminate,"panic false");P();}
void t7(){T(json_object);auto j=CppErrorModelMapping::toJson(CppErrorModelMapping::map(sample()));C(j.is_object(),"not object");P();}
void t8(){T(json_has_resultType);auto j=CppErrorModelMapping::toJson(CppErrorModelMapping::map(sample()));C(j.contains("resultType"),"missing");P();}
void t9(){T(json_has_panic_flag);auto j=CppErrorModelMapping::toJson(CppErrorModelMapping::map(sample()));C(j.contains("panicTranslatedToTerminate"),"missing");P();}
void t10(){T(deterministic);auto a=CppErrorModelMapping::toJson(CppErrorModelMapping::map(sample())).dump();auto b=CppErrorModelMapping::toJson(CppErrorModelMapping::map(sample())).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 713: Error model mapping\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
