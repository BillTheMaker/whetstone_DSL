// Step 770: Lisp lowering/raising adapters (10 tests)
#include "ast_native/LispAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_lisp);auto x=LispAdapterV1::lower("(x)");C(x.sourceLanguage=="lisp","lang");P();}
void t2(){T(canonical_form);auto x=LispAdapterV1::lower("(x)");C(x.canonicalForm=="sexpr_ir_v1","form");P();}
void t3(){T(macro_detected);auto x=LispAdapterV1::lower("(defmacro m () nil)");C(x.macroLike,"macro");P();}
void t4(){T(list_depth);auto x=LispAdapterV1::lower("(x)");C(x.listDepth==1,"depth");P();}
void t5(){T(raise_target);auto x=LispAdapterV1::raise("sexpr_ir_v1","strict");C(x.targetLanguage=="lisp","target");P();}
void t6(){T(raise_style_hygienic);auto x=LispAdapterV1::raise("sexpr_ir_v1","strict");C(x.projectionStyle=="hygienic","style");P();}
void t7(){T(raise_style_canonical);auto x=LispAdapterV1::raise("sexpr_ir_v1","balanced");C(x.projectionStyle=="canonical","style");P();}
void t8(){T(json_lower_shape);auto j=LispAdapterV1::toJson(LispAdapterV1::lower("(x)"));C(j.contains("source_language")&&j.contains("macro_like"),"shape");P();}
void t9(){T(json_raise_shape);auto j=LispAdapterV1::toJson(LispAdapterV1::raise("sexpr_ir_v1","strict"));C(j.contains("target_language")&&j.contains("projection_style"),"shape");P();}
void t10(){T(deterministic);auto a=LispAdapterV1::toJson(LispAdapterV1::lower("(defmacro m () nil)")).dump();auto b=LispAdapterV1::toJson(LispAdapterV1::lower("(defmacro m () nil)")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 770: LispAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
