// Step 775: Eval/runtime boundary risk model (8 tests)
#include "ast_native/EvalRuntimeRiskModel.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(low_without_eval_or_load);auto x=EvalRuntimeRiskModel::classify("(define x 1)");C(x.level=="low","level");P();}
void t2(){T(medium_with_eval);auto x=EvalRuntimeRiskModel::classify("(eval x)");C(x.level=="medium","level");P();}
void t3(){T(medium_with_load);auto x=EvalRuntimeRiskModel::classify("(load file)");C(x.level=="medium","level");P();}
void t4(){T(high_with_eval_and_load);auto x=EvalRuntimeRiskModel::classify("(eval x) (load file)");C(x.level=="high","level");P();}
void t5(){T(reasons_non_empty);auto x=EvalRuntimeRiskModel::classify("(define x 1)");C(!x.reasons.empty(),"reasons");P();}
void t6(){T(reasons_include_eval);auto x=EvalRuntimeRiskModel::classify("(eval x)");bool ok=false;for(const auto& r:x.reasons)if(r=="eval_present")ok=true;C(ok,"reason");P();}
void t7(){T(json_shape);auto j=EvalRuntimeRiskModel::toJson(EvalRuntimeRiskModel::classify("(eval x)"));C(j.contains("level")&&j.contains("reasons"),"shape");P();}
void t8(){T(deterministic);auto a=EvalRuntimeRiskModel::toJson(EvalRuntimeRiskModel::classify("(eval x) (load file)")).dump();auto b=EvalRuntimeRiskModel::toJson(EvalRuntimeRiskModel::classify("(eval x) (load file)")).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 775: EvalRuntimeRiskModel\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
