// Step 709: ownership mapping policy engine (12 tests)

#include "cpp_ir/CppOwnershipMappingPolicy.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static SemanticCoreIR sample() {
    SemanticCoreIR ir;
    ir.moduleId = "m";
    ir.nodes.push_back({"t1", IRNodeKind::Type, "A", "rust", {"shared"}, nlohmann::json::object(), nlohmann::json::object()});
    ir.nodes.push_back({"o1", IRNodeKind::OwnershipRegion, "Owner", "rust", {"stateful"}, nlohmann::json::object(), nlohmann::json::object()});
    ir.nodes.push_back({"t2", IRNodeKind::Type, "B", "rust", {}, nlohmann::json::object(), nlohmann::json::object()});
    return ir;
}

void t1(){T(map_non_empty);auto d=CppOwnershipMappingPolicy::map(sample());C(!d.empty(),"empty");P();}
void t2(){T(shared_ptr_for_shared_tag);auto d=CppOwnershipMappingPolicy::map(sample());bool ok=false;for(auto&x:d)if(x.nodeId=="t1"&&x.strategy=="shared_ptr")ok=true;C(ok,"shared rule");P();}
void t3(){T(unique_ptr_for_stateful);auto d=CppOwnershipMappingPolicy::map(sample());bool ok=false;for(auto&x:d)if(x.nodeId=="o1"&&x.strategy=="unique_ptr")ok=true;C(ok,"unique rule");P();}
void t4(){T(safe_first_defaults_unique);auto d=CppOwnershipMappingPolicy::map(sample(),"safe-first");bool ok=false;for(auto&x:d)if(x.nodeId=="t2"&&x.strategy=="unique_ptr")ok=true;C(ok,"safe default");P();}
void t5(){T(perf_first_defaults_value);auto d=CppOwnershipMappingPolicy::map(sample(),"perf-first");bool ok=false;for(auto&x:d)if(x.nodeId=="t2"&&x.strategy=="value")ok=true;C(ok,"perf default");P();}
void t6(){T(interop_overrides_unique_to_value);auto d=CppOwnershipMappingPolicy::map(sample(),"interop-first");bool ok=false;for(auto&x:d)if(x.nodeId=="o1"&&x.strategy=="value")ok=true;C(ok,"interop override");P();}
void t7(){T(reason_non_empty);auto d=CppOwnershipMappingPolicy::map(sample());C(!d[0].reason.empty(),"reason empty");P();}
void t8(){T(json_array_export);auto j=CppOwnershipMappingPolicy::toJson(CppOwnershipMappingPolicy::map(sample()));C(j.is_array(),"not array");P();}
void t9(){T(json_has_strategy);auto j=CppOwnershipMappingPolicy::toJson(CppOwnershipMappingPolicy::map(sample()));C(j[0].contains("strategy"),"strategy missing");P();}
void t10(){T(deterministic_output);auto a=CppOwnershipMappingPolicy::toJson(CppOwnershipMappingPolicy::map(sample())).dump();auto b=CppOwnershipMappingPolicy::toJson(CppOwnershipMappingPolicy::map(sample())).dump();C(a==b,"nondeterministic");P();}
void t11(){T(sorted_by_node_id);auto d=CppOwnershipMappingPolicy::map(sample());C(d.size()<2||d[0].nodeId<=d[1].nodeId,"unsorted");P();}
void t12(){T(empty_ir_empty_output);SemanticCoreIR ir;ir.moduleId="m";auto d=CppOwnershipMappingPolicy::map(ir);C(d.empty(),"expected empty");P();}

int main(){std::cout<<"Step 709: Cpp ownership mapping\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
