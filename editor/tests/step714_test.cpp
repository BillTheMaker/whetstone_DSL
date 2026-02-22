// Step 714: async mapping strategy (10 tests)

#include "cpp_ir/CppAsyncMappingStrategy.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static SemanticCoreIR sample(){SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"f",IRNodeKind::Function,"run","rust",{"async"},nlohmann::json::object(),nlohmann::json::object()});ir.nodes.push_back({"c",IRNodeKind::ConcurrencyRegion,"task","rust",{"async"},nlohmann::json::object(),nlohmann::json::object()});return ir;}

void t1(){T(async_nodes_detected);auto d=CppAsyncMappingStrategy::map(sample());C(d.size()==2,"size");P();}
void t2(){T(default_coroutine_std);auto d=CppAsyncMappingStrategy::map(sample(),"safe-first");C(d[0].runtimeProfile=="coroutine_std","profile");P();}
void t3(){T(perf_coroutine_std);auto d=CppAsyncMappingStrategy::map(sample(),"perf-first");C(d[0].runtimeProfile=="coroutine_std","profile");P();}
void t4(){T(interop_task_runtime);auto d=CppAsyncMappingStrategy::map(sample(),"interop-first");C(d[0].runtimeProfile=="task_runtime","profile");P();}
void t5(){T(non_async_ignored);SemanticCoreIR ir;ir.moduleId="m";ir.nodes.push_back({"x",IRNodeKind::Function,"x","rust",{},nlohmann::json::object(),nlohmann::json::object()});C(CppAsyncMappingStrategy::map(ir).empty(),"ignore");P();}
void t6(){T(json_array);auto j=CppAsyncMappingStrategy::toJson(CppAsyncMappingStrategy::map(sample()));C(j.is_array(),"array");P();}
void t7(){T(json_has_runtime_profile);auto j=CppAsyncMappingStrategy::toJson(CppAsyncMappingStrategy::map(sample()));C(j[0].contains("runtimeProfile"),"missing");P();}
void t8(){T(deterministic);auto a=CppAsyncMappingStrategy::toJson(CppAsyncMappingStrategy::map(sample())).dump();auto b=CppAsyncMappingStrategy::toJson(CppAsyncMappingStrategy::map(sample())).dump();C(a==b,"nondeterministic");P();}
void t9(){T(sorted);auto d=CppAsyncMappingStrategy::map(sample());C(d.size()<2||d[0].nodeId<=d[1].nodeId,"sort");P();}
void t10(){T(empty_ir);SemanticCoreIR ir;ir.moduleId="m";C(CppAsyncMappingStrategy::map(ir).empty(),"empty");P();}

int main(){std::cout<<"Step 714: Async mapping\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
