// Step 771: Scheme lowering/raising adapters (10 tests)
#include "ast_native/SchemeAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_scheme);auto x=SchemeAdapterV1::lower("(x)");C(x.sourceLanguage=="scheme","lang");P();}
void t2(){T(canonical_form);auto x=SchemeAdapterV1::lower("(x)");C(x.canonicalForm=="sexpr_ir_v1","form");P();}
void t3(){T(non_macro_by_default);auto x=SchemeAdapterV1::lower("(define x 1)");C(!x.macroLike,"macro");P();}
void t4(){T(list_depth);auto x=SchemeAdapterV1::lower("(x)");C(x.listDepth==1,"depth");P();}
void t5(){T(raise_target);auto x=SchemeAdapterV1::raise("sexpr_ir_v1","strict");C(x.targetLanguage=="scheme","target");P();}
void t6(){T(raise_style_hygienic);auto x=SchemeAdapterV1::raise("sexpr_ir_v1","strict");C(x.projectionStyle=="hygienic","style");P();}
void t7(){T(raise_style_canonical);auto x=SchemeAdapterV1::raise("sexpr_ir_v1","balanced");C(x.projectionStyle=="canonical","style");P();}
void t8(){T(json_lower_shape);auto j=SchemeAdapterV1::toJson(SchemeAdapterV1::lower("(x)"));C(j.contains("source_language")&&j.contains("list_depth"),"shape");P();}
void t9(){T(json_raise_shape);auto j=SchemeAdapterV1::toJson(SchemeAdapterV1::raise("sexpr_ir_v1","strict"));C(j.contains("target_language")&&j.contains("projection_style"),"shape");P();}
void t10(){T(deterministic);auto a=SchemeAdapterV1::toJson(SchemeAdapterV1::lower("(x)")).dump();auto b=SchemeAdapterV1::toJson(SchemeAdapterV1::lower("(x)")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 771: SchemeAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
