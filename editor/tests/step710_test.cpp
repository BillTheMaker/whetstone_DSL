// Step 710: borrow semantics mapping (10 tests)

#include "cpp_ir/CppBorrowMappingStrategy.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static SemanticCoreIR sample(){SemanticCoreIR ir;ir.moduleId="m";ir.edges.push_back({"a","b","borrows"});ir.edges.push_back({"a","c","borrows_mut"});return ir;}

void t1(){T(map_non_empty);auto d=CppBorrowMappingStrategy::map(sample());C(d.size()==2,"size");P();}
void t2(){T(shared_borrow_to_const_ref);auto d=CppBorrowMappingStrategy::map(sample(),"safe-first");bool ok=false;for(auto&x:d)if(x.toId=="b"&&x.cppForm=="const_ref")ok=true;C(ok,"const ref");P();}
void t3(){T(mut_borrow_to_mut_ref);auto d=CppBorrowMappingStrategy::map(sample());bool ok=false;for(auto&x:d)if(x.toId=="c"&&x.cppForm=="mut_ref")ok=true;C(ok,"mut ref");P();}
void t4(){T(perf_shared_borrow_to_span);auto d=CppBorrowMappingStrategy::map(sample(),"perf-first");bool ok=false;for(auto&x:d)if(x.toId=="b"&&x.cppForm=="span_view")ok=true;C(ok,"span view");P();}
void t5(){T(non_borrow_edges_ignored);SemanticCoreIR ir;ir.moduleId="m";ir.edges.push_back({"x","y","calls"});auto d=CppBorrowMappingStrategy::map(ir);C(d.empty(),"should ignore");P();}
void t6(){T(json_array);auto j=CppBorrowMappingStrategy::toJson(CppBorrowMappingStrategy::map(sample()));C(j.is_array(),"not array");P();}
void t7(){T(json_has_cppForm);auto j=CppBorrowMappingStrategy::toJson(CppBorrowMappingStrategy::map(sample()));C(j[0].contains("cppForm"),"cppForm missing");P();}
void t8(){T(deterministic);auto a=CppBorrowMappingStrategy::toJson(CppBorrowMappingStrategy::map(sample())).dump();auto b=CppBorrowMappingStrategy::toJson(CppBorrowMappingStrategy::map(sample())).dump();C(a==b,"nondeterministic");P();}
void t9(){T(sorted);auto d=CppBorrowMappingStrategy::map(sample());C(d.size()<2||(d[0].fromId<d[1].fromId||d[0].toId<=d[1].toId),"unsorted");P();}
void t10(){T(empty_ir);SemanticCoreIR ir;ir.moduleId="m";C(CppBorrowMappingStrategy::map(ir).empty(),"expected empty");P();}

int main(){std::cout<<"Step 710: Cpp borrow mapping\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
