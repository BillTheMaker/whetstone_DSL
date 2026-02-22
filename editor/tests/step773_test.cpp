// Step 773: Smalltalk lowering/raising adapters (10 tests)
#include "ast_native/SmalltalkAdapterV1.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(lang_smalltalk);auto x=SmalltalkAdapterV1::lower("Object new");C(x.sourceLanguage=="smalltalk","lang");P();}
void t2(){T(canonical_form);auto x=SmalltalkAdapterV1::lower("Object new");C(x.canonicalForm=="sexpr_ir_v1","form");P();}
void t3(){T(message_send_like_true);auto x=SmalltalkAdapterV1::lower("obj do: x");C(x.messageSendLike,"msg");P();}
void t4(){T(list_depth_non_zero);auto x=SmalltalkAdapterV1::lower("Object new");C(x.listDepth==1,"depth");P();}
void t5(){T(raise_target);auto x=SmalltalkAdapterV1::raise("sexpr_ir_v1","strict");C(x.targetLanguage=="smalltalk","target");P();}
void t6(){T(raise_style_message_safe);auto x=SmalltalkAdapterV1::raise("sexpr_ir_v1","strict");C(x.projectionStyle=="message_safe","style");P();}
void t7(){T(raise_style_message_friendly);auto x=SmalltalkAdapterV1::raise("sexpr_ir_v1","balanced");C(x.projectionStyle=="message_friendly","style");P();}
void t8(){T(json_lower_shape);auto j=SmalltalkAdapterV1::toJson(SmalltalkAdapterV1::lower("Object new"));C(j.contains("source_language")&&j.contains("message_send_like"),"shape");P();}
void t9(){T(json_raise_shape);auto j=SmalltalkAdapterV1::toJson(SmalltalkAdapterV1::raise("sexpr_ir_v1","strict"));C(j.contains("target_language")&&j.contains("projection_style"),"shape");P();}
void t10(){T(deterministic);auto a=SmalltalkAdapterV1::toJson(SmalltalkAdapterV1::lower("obj do: x")).dump();auto b=SmalltalkAdapterV1::toJson(SmalltalkAdapterV1::lower("obj do: x")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 773: SmalltalkAdapterV1\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
