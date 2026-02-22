// Step 777: AST-native projection benchmark suite (8 tests)
#include "ast_native/ASTNativeProjectionBenchmark.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(corpus_count_set);auto b=ASTNativeProjectionBenchmarkSuite::run(10,1);C(b.corpusCount==10,"count");P();}
void t2(){T(review_rate_calc);auto b=ASTNativeProjectionBenchmarkSuite::run(10,2);C(b.reviewRequiredRate==0.2,"rate");P();}
void t3(){T(confidence_inverse_rate);auto b=ASTNativeProjectionBenchmarkSuite::run(10,2);C(b.equivalenceConfidence==0.8,"conf");P();}
void t4(){T(zero_total_defaults);auto b=ASTNativeProjectionBenchmarkSuite::run(0,0);C(b.reviewRequiredRate==1.0&&b.equivalenceConfidence==0.0,"zero");P();}
void t5(){T(low_review_high_confidence);auto b=ASTNativeProjectionBenchmarkSuite::run(10,1);C(b.equivalenceConfidence>0.5,"conf");P();}
void t6(){T(json_shape);auto j=ASTNativeProjectionBenchmarkSuite::toJson(ASTNativeProjectionBenchmarkSuite::run(10,1));C(j.contains("equivalence_confidence")&&j.contains("review_required_rate"),"shape");P();}
void t7(){T(machine_readable);auto j=ASTNativeProjectionBenchmarkSuite::toJson(ASTNativeProjectionBenchmarkSuite::run(10,1));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=ASTNativeProjectionBenchmarkSuite::toJson(ASTNativeProjectionBenchmarkSuite::run(10,1)).dump();auto b=ASTNativeProjectionBenchmarkSuite::toJson(ASTNativeProjectionBenchmarkSuite::run(10,1)).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 777: ASTNativeProjectionBenchmark\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
