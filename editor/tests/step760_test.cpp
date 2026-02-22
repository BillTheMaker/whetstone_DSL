// Step 760: C# lowering/raising adapters (10 tests)
#include "managed/CSharpAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_csharp);auto x=CSharpAdapterV1::lower("class C{}");C(x.sourceLanguage=="csharp","lang");P();}
void t2(){T(non_empty_ir);auto x=CSharpAdapterV1::lower("class C{}");C(x.irSummary=="csharp_ir_v1","ir");P();}
void t3(){T(nullable_detected);auto x=CSharpAdapterV1::lower("string? s = null;");C(x.hasNullableSyntax,"nullable");P();}
void t4(){T(async_detected);auto x=CSharpAdapterV1::lower("async Task Go(){}");C(x.asyncSignalCount==1,"async");P();}
void t5(){T(adt_detected);auto x=CSharpAdapterV1::lower("record R(int X); switch(x){}");C(x.adtLike,"adt");P();}
void t6(){T(raise_target);auto x=CSharpAdapterV1::raise("csharp_ir_v1","strict");C(x.targetLanguage=="csharp","target");P();}
void t7(){T(raise_nrt_strict);auto x=CSharpAdapterV1::raise("csharp_ir_v1","strict");C(x.nullabilityModel=="csharp_nrt_enabled","nrt");P();}
void t8(){T(raise_task_model);auto x=CSharpAdapterV1::raise("csharp_ir_v1","balanced");C(x.asyncModel=="task","task");P();}
void t9(){T(json_shape);auto j=CSharpAdapterV1::toJson(CSharpAdapterV1::lower("class C{}"));C(j.contains("source_language")&&j.contains("async_signal_count"),"shape");P();}
void t10(){T(deterministic);auto a=CSharpAdapterV1::toJson(CSharpAdapterV1::lower("async Task Go(){}")).dump();auto b=CSharpAdapterV1::toJson(CSharpAdapterV1::lower("async Task Go(){}")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 760: CSharpAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
