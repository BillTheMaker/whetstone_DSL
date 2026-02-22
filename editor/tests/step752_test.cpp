// Step 752: Ruby lowering adapter v1 (10 tests)
#include "dynamic/RubyAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_ruby);auto r=RubyLoweringAdapterV1::lower("x");C(r.sourceLanguage=="ruby","lang");P();}
void t2(){T(non_empty_ir);auto r=RubyLoweringAdapterV1::lower("x");C(r.irSummary=="ruby_ir_v1","ir");P();}
void t3(){T(empty_ir);auto r=RubyLoweringAdapterV1::lower("");C(r.irSummary=="empty_ruby_unit","ir");P();}
void t4(){T(method_missing_detected);auto r=RubyLoweringAdapterV1::lower("method_missing");C(r.dynamicDispatchCount==1,"disp");P();}
void t5(){T(review_required);auto r=RubyLoweringAdapterV1::lower("method_missing");C(r.reviewRequired,"rev");P();}
void t6(){T(json_shape);auto j=RubyLoweringAdapterV1::toJson(RubyLoweringAdapterV1::lower("x"));C(j.contains("source_language")&&j.contains("ir_summary")&&j.contains("dynamic_dispatch_count")&&j.contains("review_required"),"shape");P();}
void t7(){T(json_lang);auto j=RubyLoweringAdapterV1::toJson(RubyLoweringAdapterV1::lower("x"));C(j.value("source_language","")=="ruby","lang");P();}
void t8(){T(machine_readable);auto j=RubyLoweringAdapterV1::toJson(RubyLoweringAdapterV1::lower("x"));C(j.is_object(),"obj");P();}
void t9(){T(deterministic);auto a=RubyLoweringAdapterV1::toJson(RubyLoweringAdapterV1::lower("method_missing")).dump();auto b=RubyLoweringAdapterV1::toJson(RubyLoweringAdapterV1::lower("method_missing")).dump();C(a==b,"det");P();}
void t10(){T(no_pattern_zero_dispatch);auto r=RubyLoweringAdapterV1::lower("puts x");C(r.dynamicDispatchCount==0,"disp");P();}
int main(){std::cout<<"Step 752: RubyLoweringAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
