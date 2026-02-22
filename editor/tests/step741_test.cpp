// Step 741: Java lowering adapter v1 (10 tests)
#include "systems/JavaAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(source_lang_java);auto r=JavaLoweringAdapterV1::lower("x");C(r.sourceLanguage=="java","lang");P();}
void t2(){T(empty_source_path);auto r=JavaLoweringAdapterV1::lower("");C(r.irSummary=="empty_java_unit","empty");P();}
void t3(){T(non_empty_ir);auto r=JavaLoweringAdapterV1::lower("x");C(r.irSummary=="java_ir_v1","ir");P();}
void t4(){T(confidence_positive);auto r=JavaLoweringAdapterV1::lower("x");C(r.confidence>0.0,"conf");P();}
void t5(){T(to_json_shape);auto j=JavaLoweringAdapterV1::toJson(JavaLoweringAdapterV1::lower("x"));C(j.contains("source_language")&&j.contains("ir_summary")&&j.contains("confidence"),"shape");P();}
void t6(){T(json_lang_java);auto j=JavaLoweringAdapterV1::toJson(JavaLoweringAdapterV1::lower("x"));C(j.value("source_language","")=="java","lang");P();}
void t7(){T(machine_readable);auto j=JavaLoweringAdapterV1::toJson(JavaLoweringAdapterV1::lower("x"));C(j.is_object(),"obj");P();}
void t8(){T(deterministic_non_empty);auto a=JavaLoweringAdapterV1::toJson(JavaLoweringAdapterV1::lower("x")).dump();auto b=JavaLoweringAdapterV1::toJson(JavaLoweringAdapterV1::lower("x")).dump();C(a==b,"det");P();}
void t9(){T(deterministic_empty);auto a=JavaLoweringAdapterV1::toJson(JavaLoweringAdapterV1::lower("")).dump();auto b=JavaLoweringAdapterV1::toJson(JavaLoweringAdapterV1::lower("")).dump();C(a==b,"det");P();}
void t10(){T(confidence_bounds);auto r=JavaLoweringAdapterV1::lower("x");C(r.confidence>=0.0&&r.confidence<=1.0,"bounds");P();}
int main(){std::cout<<"Step 741: JavaLoweringAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
