// Step 769: S-expression canonical lowering layer (12 tests)
#include "ast_native/SExpressionCanonicalLowering.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(language_preserved);auto x=SExpressionCanonicalLowering::lower("(x)","lisp");C(x.sourceLanguage=="lisp","lang");P();}
void t2(){T(non_empty_form);auto x=SExpressionCanonicalLowering::lower("(x)","lisp");C(x.canonicalForm=="sexpr_ir_v1","form");P();}
void t3(){T(empty_form);auto x=SExpressionCanonicalLowering::lower("","lisp");C(x.canonicalForm=="empty_form","form");P();}
void t4(){T(list_depth_detected);auto x=SExpressionCanonicalLowering::lower("(x)","lisp");C(x.listDepth==1,"depth");P();}
void t5(){T(list_depth_zero);auto x=SExpressionCanonicalLowering::lower("x","lisp");C(x.listDepth==0,"depth");P();}
void t6(){T(macro_detected);auto x=SExpressionCanonicalLowering::lower("(defmacro m () nil)","lisp");C(x.macroLike,"macro");P();}
void t7(){T(macro_not_detected);auto x=SExpressionCanonicalLowering::lower("(define x 1)","scheme");C(!x.macroLike,"macro");P();}
void t8(){T(message_like_detected);auto x=SExpressionCanonicalLowering::lower("obj do: x","smalltalk");C(x.messageSendLike,"msg");P();}
void t9(){T(raise_target);auto x=SExpressionCanonicalLowering::raise("sexpr_ir_v1","scheme","canonical");C(x.targetLanguage=="scheme","target");P();}
void t10(){T(raise_style_default);auto x=SExpressionCanonicalLowering::raise("sexpr_ir_v1","scheme","");C(x.projectionStyle=="canonical","style");P();}
void t11(){T(json_shape);auto j=SExpressionCanonicalLowering::toJson(SExpressionCanonicalLowering::lower("(x)","lisp"));C(j.contains("source_language")&&j.contains("canonical_form"),"shape");P();}
void t12(){T(deterministic);auto a=SExpressionCanonicalLowering::toJson(SExpressionCanonicalLowering::lower("(x)","lisp")).dump();auto b=SExpressionCanonicalLowering::toJson(SExpressionCanonicalLowering::lower("(x)","lisp")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 769: SExpressionCanonicalLowering\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
