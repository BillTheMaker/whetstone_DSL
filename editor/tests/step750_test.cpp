// Step 750: JavaScript lowering adapter v2 (10 tests)
#include "dynamic/JavaScriptAdapterV2.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_js);auto r=JavaScriptLoweringAdapterV2::lower("x");C(r.sourceLanguage=="javascript","lang");P();}
void t2(){T(non_empty_ir);auto r=JavaScriptLoweringAdapterV2::lower("x");C(r.irSummary=="js_ir_v2","ir");P();}
void t3(){T(empty_ir);auto r=JavaScriptLoweringAdapterV2::lower("");C(r.irSummary=="empty_js_unit","ir");P();}
void t4(){T(dispatch_detected);auto r=JavaScriptLoweringAdapterV2::lower("obj[k]");C(r.dynamicDispatchCount==1,"disp");P();}
void t5(){T(review_required);auto r=JavaScriptLoweringAdapterV2::lower("obj[k]");C(r.reviewRequired,"rev");P();}
void t6(){T(json_shape);auto j=JavaScriptLoweringAdapterV2::toJson(JavaScriptLoweringAdapterV2::lower("x"));C(j.contains("source_language")&&j.contains("ir_summary")&&j.contains("dynamic_dispatch_count")&&j.contains("review_required"),"shape");P();}
void t7(){T(json_lang);auto j=JavaScriptLoweringAdapterV2::toJson(JavaScriptLoweringAdapterV2::lower("x"));C(j.value("source_language","")=="javascript","lang");P();}
void t8(){T(machine_readable);auto j=JavaScriptLoweringAdapterV2::toJson(JavaScriptLoweringAdapterV2::lower("x"));C(j.is_object(),"obj");P();}
void t9(){T(deterministic);auto a=JavaScriptLoweringAdapterV2::toJson(JavaScriptLoweringAdapterV2::lower("obj[k]")).dump();auto b=JavaScriptLoweringAdapterV2::toJson(JavaScriptLoweringAdapterV2::lower("obj[k]")).dump();C(a==b,"det");P();}
void t10(){T(dispatch_zero_no_brackets);auto r=JavaScriptLoweringAdapterV2::lower("obj.k");C(r.dynamicDispatchCount==0,"disp");P();}
int main(){std::cout<<"Step 750: JavaScriptLoweringAdapterV2\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
