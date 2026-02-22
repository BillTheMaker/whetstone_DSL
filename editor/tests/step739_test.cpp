// Step 739: C lowering adapter v1 (12 tests)
#include "systems/CAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(source_lang_c);auto r=CLoweringAdapterV1::lower("x");C(r.sourceLanguage=="c","lang");P();}
void t2(){T(non_empty_ir_summary);auto r=CLoweringAdapterV1::lower("x");C(!r.irSummary.empty(),"ir");P();}
void t3(){T(empty_source_path);auto r=CLoweringAdapterV1::lower("");C(r.irSummary=="empty_c_unit","empty");P();}
void t4(){T(confidence_non_empty);auto r=CLoweringAdapterV1::lower("x");C(r.confidence>0.0,"conf");P();}
void t5(){T(confidence_lower_on_empty);auto a=CLoweringAdapterV1::lower("");auto b=CLoweringAdapterV1::lower("x");C(a.confidence<b.confidence,"conf");P();}
void t6(){T(to_json_shape);auto j=CLoweringAdapterV1::toJson(CLoweringAdapterV1::lower("x"));C(j.contains("source_language")&&j.contains("ir_summary")&&j.contains("confidence"),"shape");P();}
void t7(){T(json_lang_c);auto j=CLoweringAdapterV1::toJson(CLoweringAdapterV1::lower("x"));C(j.value("source_language","")=="c","lang");P();}
void t8(){T(machine_readable);auto j=CLoweringAdapterV1::toJson(CLoweringAdapterV1::lower("x"));C(j.is_object(),"obj");P();}
void t9(){T(deterministic_non_empty);auto a=CLoweringAdapterV1::toJson(CLoweringAdapterV1::lower("x")).dump();auto b=CLoweringAdapterV1::toJson(CLoweringAdapterV1::lower("x")).dump();C(a==b,"det");P();}
void t10(){T(deterministic_empty);auto a=CLoweringAdapterV1::toJson(CLoweringAdapterV1::lower("")).dump();auto b=CLoweringAdapterV1::toJson(CLoweringAdapterV1::lower("")).dump();C(a==b,"det");P();}
void t11(){T(confidence_bounds);auto r=CLoweringAdapterV1::lower("x");C(r.confidence>=0.0&&r.confidence<=1.0,"bounds");P();}
void t12(){T(ir_summary_expected);auto r=CLoweringAdapterV1::lower("x");C(r.irSummary=="c_ir_v1","ir");P();}
int main(){std::cout<<"Step 739: CLoweringAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
