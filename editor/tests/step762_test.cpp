// Step 762: VB.NET lowering/raising adapters (10 tests)
#include "managed/VbNetAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_vbnet);auto x=VbNetAdapterV1::lower("Class C\nEnd Class");C(x.sourceLanguage=="vbnet","lang");P();}
void t2(){T(non_empty_ir);auto x=VbNetAdapterV1::lower("Class C\nEnd Class");C(x.irSummary=="vbnet_ir_v1","ir");P();}
void t3(){T(nullable_detected);auto x=VbNetAdapterV1::lower("Dim s As String = Nothing");C(x.hasNullableSyntax,"nullable");P();}
void t4(){T(async_detected);auto x=VbNetAdapterV1::lower("Public Async Function Go() As Task\nEnd Function");C(x.asyncSignalCount==1,"async");P();}
void t5(){T(select_case_detected);auto x=VbNetAdapterV1::lower("Select Case x\nEnd Select");C(x.adtLike,"adt");P();}
void t6(){T(raise_target);auto x=VbNetAdapterV1::raise("vbnet_ir_v1","strict");C(x.targetLanguage=="vbnet","target");P();}
void t7(){T(raise_strict_model);auto x=VbNetAdapterV1::raise("vbnet_ir_v1","strict");C(x.nullabilityModel=="vbnet_nullable_strict","model");P();}
void t8(){T(raise_async_model);auto x=VbNetAdapterV1::raise("vbnet_ir_v1","balanced");C(x.asyncModel=="task","async");P();}
void t9(){T(json_shape);auto j=VbNetAdapterV1::toJson(VbNetAdapterV1::lower("Class C\nEnd Class"));C(j.contains("source_language")&&j.contains("adt_like"),"shape");P();}
void t10(){T(deterministic);auto a=VbNetAdapterV1::toJson(VbNetAdapterV1::lower("Select Case x\nEnd Select")).dump();auto b=VbNetAdapterV1::toJson(VbNetAdapterV1::lower("Select Case x\nEnd Select")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 762: VbNetAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
