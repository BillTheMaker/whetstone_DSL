// Step 749: Python lowering adapter v2 (12 tests)
#include "dynamic/PythonAdapterV2.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_python);auto r=PythonLoweringAdapterV2::lower("x");C(r.sourceLanguage=="python","lang");P();}
void t2(){T(non_empty_ir);auto r=PythonLoweringAdapterV2::lower("x");C(r.irSummary=="python_ir_v2","ir");P();}
void t3(){T(empty_source_ir);auto r=PythonLoweringAdapterV2::lower("");C(r.irSummary=="empty_python_unit","ir");P();}
void t4(){T(dispatch_detected);auto r=PythonLoweringAdapterV2::lower("getattr(x,'a')");C(r.dynamicDispatchCount==1,"disp");P();}
void t5(){T(dispatch_zero_without_pattern);auto r=PythonLoweringAdapterV2::lower("x.a");C(r.dynamicDispatchCount==0,"disp");P();}
void t6(){T(review_required_when_dispatch);auto r=PythonLoweringAdapterV2::lower("getattr(x,'a')");C(r.reviewRequired,"rev");P();}
void t7(){T(review_not_required_without_dispatch);auto r=PythonLoweringAdapterV2::lower("x.a");C(!r.reviewRequired,"rev");P();}
void t8(){T(json_shape);auto j=PythonLoweringAdapterV2::toJson(PythonLoweringAdapterV2::lower("x"));C(j.contains("source_language")&&j.contains("ir_summary")&&j.contains("dynamic_dispatch_count")&&j.contains("review_required"),"shape");P();}
void t9(){T(json_lang);auto j=PythonLoweringAdapterV2::toJson(PythonLoweringAdapterV2::lower("x"));C(j.value("source_language","")=="python","lang");P();}
void t10(){T(machine_readable);auto j=PythonLoweringAdapterV2::toJson(PythonLoweringAdapterV2::lower("x"));C(j.is_object(),"obj");P();}
void t11(){T(deterministic_non_empty);auto a=PythonLoweringAdapterV2::toJson(PythonLoweringAdapterV2::lower("x")).dump();auto b=PythonLoweringAdapterV2::toJson(PythonLoweringAdapterV2::lower("x")).dump();C(a==b,"det");P();}
void t12(){T(deterministic_dispatch);auto a=PythonLoweringAdapterV2::toJson(PythonLoweringAdapterV2::lower("getattr(x,'a')")).dump();auto b=PythonLoweringAdapterV2::toJson(PythonLoweringAdapterV2::lower("getattr(x,'a')")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 749: PythonLoweringAdapterV2\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
