// Step 715: STL algorithm lifting (8 tests)

#include "cpp_ir/CppAlgorithmLifting.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static SemanticCoreIR sample(){SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"f",IRNodeKind::Function,"run","rust",{"algorithmic","pure"},nlohmann::json::object(),nlohmann::json::object()});ir.nodes.push_back({"io",IRNodeKind::Effect,"io","rust",{"io"},nlohmann::json::object(),nlohmann::json::object()});return ir;}

void t1(){T(lifts_transform);auto s=CppAlgorithmLifting::lift(sample());C(s.count("std::transform")==1,"transform");P();}
void t2(){T(lifts_accumulate);auto s=CppAlgorithmLifting::lift(sample());C(s.count("std::accumulate")==1,"accumulate");P();}
void t3(){T(lifts_for_each);auto s=CppAlgorithmLifting::lift(sample());C(s.count("std::for_each")==1,"for_each");P();}
void t4(){T(unique_algorithms);auto s=CppAlgorithmLifting::lift(sample());C(s.size()==3,"size");P();}
void t5(){T(empty_for_no_tags);SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"x",IRNodeKind::Function,"x","rust",{},nlohmann::json::object(),nlohmann::json::object()});C(CppAlgorithmLifting::lift(ir).empty(),"should empty");P();}
void t6(){T(json_array);auto j=CppAlgorithmLifting::toJson(CppAlgorithmLifting::lift(sample()));C(j.is_array(),"array");P();}
void t7(){T(deterministic);auto a=CppAlgorithmLifting::toJson(CppAlgorithmLifting::lift(sample())).dump();auto b=CppAlgorithmLifting::toJson(CppAlgorithmLifting::lift(sample())).dump();C(a==b,"nondeterministic");P();}
void t8(){T(sorted_via_set);auto j=CppAlgorithmLifting::toJson(CppAlgorithmLifting::lift(sample()));C(j.size()<2 || j[0].get<std::string>()<=j[1].get<std::string>(),"order");P();}

int main(){std::cout<<"Step 715: STL algorithm lifting\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
