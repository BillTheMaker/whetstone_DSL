// Step 759: Kotlin lowering/raising adapters (12 tests)
#include "managed/KotlinAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_kotlin);auto x=KotlinAdapterV1::lower("fun f(){}");C(x.sourceLanguage=="kotlin","lang");P();}
void t2(){T(non_empty_ir);auto x=KotlinAdapterV1::lower("fun f(){}");C(x.irSummary=="kotlin_ir_v1","ir");P();}
void t3(){T(empty_ir);auto x=KotlinAdapterV1::lower("");C(x.irSummary=="empty_kotlin_unit","ir");P();}
void t4(){T(nullable_detected);auto x=KotlinAdapterV1::lower("val x: String? = null");C(x.hasNullableSyntax,"nullable");P();}
void t5(){T(async_detected);auto x=KotlinAdapterV1::lower("suspend fun go(){}");C(x.asyncSignalCount==1,"async");P();}
void t6(){T(adt_detected);auto x=KotlinAdapterV1::lower("sealed class S\nwhen(x){}");C(x.adtLike,"adt");P();}
void t7(){T(raise_target);auto x=KotlinAdapterV1::raise("kotlin_ir_v1","strict");C(x.targetLanguage=="kotlin","target");P();}
void t8(){T(raise_nullability_model);auto x=KotlinAdapterV1::raise("kotlin_ir_v1","strict");C(x.nullabilityModel=="kotlin_nullable_strict","model");P();}
void t9(){T(raise_async_model);auto x=KotlinAdapterV1::raise("kotlin_ir_v1","balanced");C(x.asyncModel=="coroutines","async");P();}
void t10(){T(json_lower_shape);auto j=KotlinAdapterV1::toJson(KotlinAdapterV1::lower("fun f(){}"));C(j.contains("source_language")&&j.contains("ir_summary"),"shape");P();}
void t11(){T(json_raise_shape);auto j=KotlinAdapterV1::toJson(KotlinAdapterV1::raise("kotlin_ir_v1","balanced"));C(j.contains("target_language")&&j.contains("code_preview"),"shape");P();}
void t12(){T(deterministic);auto a=KotlinAdapterV1::toJson(KotlinAdapterV1::lower("suspend fun go(){}")).dump();auto b=KotlinAdapterV1::toJson(KotlinAdapterV1::lower("suspend fun go(){}")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 759: KotlinAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
