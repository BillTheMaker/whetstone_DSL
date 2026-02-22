// Step 712: template raising policy (10 tests)

#include "cpp_ir/CppTemplateRaisingPolicy.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static SemanticCoreIR sample(){SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"f",IRNodeKind::Function,"f","rust",{},nlohmann::json::object(),nlohmann::json{{"genericParams",nlohmann::json::array({"T"})}}});ir.nodes.push_back({"t",IRNodeKind::Type,"Box","rust",{},nlohmann::json::object(),nlohmann::json{{"genericParams",nlohmann::json::array({"T"})}}});return ir;}

void t1(){T(generic_nodes_detected);auto v=CppTemplateRaisingPolicy::raise(sample());C(v.size()==2,"size");P();}
void t2(){T(function_to_fn_template);auto v=CppTemplateRaisingPolicy::raise(sample());bool ok=false;for(auto&d:v)if(d.nodeId=="f"&&d.templateForm=="fn_template")ok=true;C(ok,"fn template");P();}
void t3(){T(type_to_class_template);auto v=CppTemplateRaisingPolicy::raise(sample());bool ok=false;for(auto&d:v)if(d.nodeId=="t"&&d.templateForm=="class_template")ok=true;C(ok,"class template");P();}
void t4(){T(concepts_on_safe_profile);auto v=CppTemplateRaisingPolicy::raise(sample(),"safe-first");C(v[0].conceptConstrained,"concept expected");P();}
void t5(){T(concepts_off_interop);auto v=CppTemplateRaisingPolicy::raise(sample(),"interop-first");C(!v[0].conceptConstrained,"concept should be off");P();}
void t6(){T(non_generic_ignored);SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"x",IRNodeKind::Function,"x","rust",{},nlohmann::json::object(),nlohmann::json::object()});C(CppTemplateRaisingPolicy::raise(ir).empty(),"should ignore");P();}
void t7(){T(json_array);auto j=CppTemplateRaisingPolicy::toJson(CppTemplateRaisingPolicy::raise(sample()));C(j.is_array(),"not array");P();}
void t8(){T(json_has_templateForm);auto j=CppTemplateRaisingPolicy::toJson(CppTemplateRaisingPolicy::raise(sample()));C(j[0].contains("templateForm"),"missing");P();}
void t9(){T(deterministic);auto a=CppTemplateRaisingPolicy::toJson(CppTemplateRaisingPolicy::raise(sample())).dump();auto b=CppTemplateRaisingPolicy::toJson(CppTemplateRaisingPolicy::raise(sample())).dump();C(a==b,"nondeterministic");P();}
void t10(){T(sorted);auto v=CppTemplateRaisingPolicy::raise(sample());C(v.size()<2||v[0].nodeId<=v[1].nodeId,"sort");P();}

int main(){std::cout<<"Step 712: Template policy\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
