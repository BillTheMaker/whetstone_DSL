// Step 811: Build/runtime assumption inference (10 tests)
#include "legacy_ingestion/AssumptionInference.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(build_detected);auto x=AssumptionInference::analyze("Makefile");C(x.category=="build","cat");P();}
void t2(){T(runtime_detected);auto x=AssumptionInference::analyze("main.cpp");C(x.category=="runtime","cat");P();}
void t3(){T(detail_set);auto x=AssumptionInference::analyze("src");C(!x.detail.empty(),"detail");P();}
void t4(){T(runtime_flag);auto x=AssumptionInference::analyze("code");C(x.runtime,"runtime");P();}
void t5(){T(json_shape);auto j=AssumptionInference::toJson(AssumptionInference::analyze("Makefile"));C(j.contains("category")&&j.contains("runtime"),"shape");P();}
void t6(){T(machine_readable);auto j=AssumptionInference::toJson(AssumptionInference::analyze("Makefile"));C(j.is_object(),"obj");P();}
void t7(){T(deterministic);auto a=AssumptionInference::toJson(AssumptionInference::analyze(""));auto b=AssumptionInference::toJson(AssumptionInference::analyze(""));C(a==b,"det");P();}
void t8(){T(runtime_detail);auto x=AssumptionInference::analyze("src");C(x.detail.find("env")!=std::string::npos||!x.detail.empty(),"detail");P();}
void t9(){T(build_detail);auto x=AssumptionInference::analyze("Makefile");C(x.detail.find("make")!=std::string::npos||!x.detail.empty(),"detail");P();}
void t10(){T(runtime_true);auto x=AssumptionInference::analyze("run");C(x.runtime,"rt");P();}
int main(){std::cout<<"Step 811: AssumptionInference\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
