// Step 711: trait raising (10 tests)

#include "cpp_ir/CppTraitRaising.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static SemanticCoreIR sample(){SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"t","","Drawable","rust",{},nlohmann::json{{"trait",true}},nlohmann::json::object()});ir.nodes.back().kind=IRNodeKind::Type;return ir;}

void t1(){T(trait_detected);auto v=CppTraitRaising::raise(sample());C(v.size()==1,"missing");P();}
void t2(){T(name_preserved);auto v=CppTraitRaising::raise(sample());C(v[0].name=="Drawable","name");P();}
void t3(){T(safe_first_interface);auto v=CppTraitRaising::raise(sample(),"safe-first");C(v[0].kind=="interface","kind");P();}
void t4(){T(perf_first_composition);auto v=CppTraitRaising::raise(sample(),"perf-first");C(v[0].kind=="composition","kind");P();}
void t5(){T(non_trait_ignored);SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"x",IRNodeKind::Type,"X","rust",{},nlohmann::json::object(),nlohmann::json::object()});C(CppTraitRaising::raise(ir).empty(),"should ignore");P();}
void t6(){T(json_array);auto j=CppTraitRaising::toJson(CppTraitRaising::raise(sample()));C(j.is_array(),"not array");P();}
void t7(){T(json_has_kind);auto j=CppTraitRaising::toJson(CppTraitRaising::raise(sample()));C(j[0].contains("kind"),"kind missing");P();}
void t8(){T(deterministic);auto a=CppTraitRaising::toJson(CppTraitRaising::raise(sample())).dump();auto b=CppTraitRaising::toJson(CppTraitRaising::raise(sample())).dump();C(a==b,"nondeterministic");P();}
void t9(){T(sorted);SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"a",IRNodeKind::Type,"B","rust",{},nlohmann::json{{"trait",true}},nlohmann::json::object()});ir.nodes.push_back({"b",IRNodeKind::Type,"A","rust",{},nlohmann::json{{"trait",true}},nlohmann::json::object()});auto v=CppTraitRaising::raise(ir);C(v[0].name=="A","sort");P();}
void t10(){T(empty_ir);SemanticCoreIR ir;ir.moduleId="m";C(CppTraitRaising::raise(ir).empty(),"expected empty");P();}

int main(){std::cout<<"Step 711: Cpp trait raising\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
