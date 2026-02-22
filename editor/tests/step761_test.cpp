// Step 761: F# lowering/raising adapters (10 tests)
#include "managed/FSharpAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_fsharp);auto x=FSharpAdapterV1::lower("let x = 1");C(x.sourceLanguage=="fsharp","lang");P();}
void t2(){T(non_empty_ir);auto x=FSharpAdapterV1::lower("let x = 1");C(x.irSummary=="fsharp_ir_v1","ir");P();}
void t3(){T(optional_detected);auto x=FSharpAdapterV1::lower("let x : int option = None");C(x.hasOptionalType,"opt");P();}
void t4(){T(async_detected);auto x=FSharpAdapterV1::lower("let run = async { return 1 }");C(x.asyncSignalCount==1,"async");P();}
void t5(){T(adt_detected);auto x=FSharpAdapterV1::lower("type R = A | B");C(x.adtLike,"adt");P();}
void t6(){T(raise_target);auto x=FSharpAdapterV1::raise("fsharp_ir_v1","strict");C(x.targetLanguage=="fsharp","target");P();}
void t7(){T(raise_option_first);auto x=FSharpAdapterV1::raise("fsharp_ir_v1","strict");C(x.nullabilityModel=="fsharp_option_first","model");P();}
void t8(){T(raise_async_model);auto x=FSharpAdapterV1::raise("fsharp_ir_v1","balanced");C(x.asyncModel=="async_workflow","async");P();}
void t9(){T(json_shape);auto j=FSharpAdapterV1::toJson(FSharpAdapterV1::lower("let x = 1"));C(j.contains("source_language")&&j.contains("has_optional_type"),"shape");P();}
void t10(){T(deterministic);auto a=FSharpAdapterV1::toJson(FSharpAdapterV1::lower("let x : int option = None")).dump();auto b=FSharpAdapterV1::toJson(FSharpAdapterV1::lower("let x : int option = None")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 761: FSharpAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
