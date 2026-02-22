// Step 703: Error model lowering (10 tests)

#include "rust_ir/RustErrorModelLowering.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static const char* src = "fn f()->Result<Option<i32>,String>{ panic!(\"x\"); let x:Option<i32>=None; Err(\"e\".into())? }";

void t1(){T(result_detected);auto m=RustErrorModelLowering::lower(src);C(m.usesResult,"result missing");P();}
void t2(){T(option_detected);auto m=RustErrorModelLowering::lower(src);C(m.usesOption,"option missing");P();}
void t3(){T(panic_detected);auto m=RustErrorModelLowering::lower(src);C(m.usesPanic,"panic missing");P();}
void t4(){T(question_detected);auto m=RustErrorModelLowering::lower(src);C(m.usesQuestionOperator,"? missing");P();}
void t5(){T(empty_false);auto m=RustErrorModelLowering::lower("");C(!m.usesResult&&!m.usesOption&&!m.usesPanic&&!m.usesQuestionOperator,"expected false");P();}
void t6(){T(json_has_result);auto j=RustErrorModelLowering::toJson(RustErrorModelLowering::lower(src));C(j.contains("usesResult"),"usesResult missing");P();}
void t7(){T(json_has_option);auto j=RustErrorModelLowering::toJson(RustErrorModelLowering::lower(src));C(j.contains("usesOption"),"usesOption missing");P();}
void t8(){T(json_has_panic);auto j=RustErrorModelLowering::toJson(RustErrorModelLowering::lower(src));C(j.contains("usesPanic"),"usesPanic missing");P();}
void t9(){T(json_has_question);auto j=RustErrorModelLowering::toJson(RustErrorModelLowering::lower(src));C(j.contains("usesQuestionOperator"),"question missing");P();}
void t10(){T(deterministic);auto a=RustErrorModelLowering::toJson(RustErrorModelLowering::lower(src)).dump();auto b=RustErrorModelLowering::toJson(RustErrorModelLowering::lower(src)).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 703: Error model lowering\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
