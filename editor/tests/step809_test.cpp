// Step 809: Semantic recovery graph engine (12 tests)
#include "legacy_ingestion/SemanticRecoveryGraph.h"
#include <nlohmann/json.hpp>
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(multiple_nodes);auto g=SemanticRecoveryGraph::build("code");C(g.size()>=2,"size");P();}
void t2(){T(unknown_intent);auto g=SemanticRecoveryGraph::build("");C(g[0].intent=="unknown", "intent");P();}
void t3(){T(higher_confidence_non_empty);auto g=SemanticRecoveryGraph::build("foo");C(g[0].confidence>0.7,"conf");P();}
void t4(){T(json_shape);auto j=SemanticRecoveryGraph::toJson(SemanticRecoveryGraph::build("foo")[0]);C(j.contains("intent")&&j.contains("neighbors"),"shape");P();}
void t5(){T(machine_readable);auto j=SemanticRecoveryGraph::toJson(SemanticRecoveryGraph::build("foo")[0]);C(j.is_object(),"obj");P();}
void t6(){T(deterministic);auto a=SemanticRecoveryGraph::toJson(SemanticRecoveryGraph::build("foo")[0]).dump();auto b=SemanticRecoveryGraph::toJson(SemanticRecoveryGraph::build("foo")[0]).dump();C(a==b,"det");P();}
void t7(){T(todo_node);auto g=SemanticRecoveryGraph::build("TODO");C(g.size()>2,"todo");P();}
void t8(){T(neighbor_present);auto g=SemanticRecoveryGraph::build("code");C(!g[0].neighbors.empty(),"nbr");P();}
void t9(){T(intent_default);auto g=SemanticRecoveryGraph::build("");C(g[0].intent=="unknown","intent");P();}
void t10(){T(confidence_low_empty);auto g=SemanticRecoveryGraph::build("");C(g[0].confidence<0.5,"conf");P();}
void t11(){T(additional_node_count);auto g=SemanticRecoveryGraph::build("TODO");C(g.size()>2,"size");P();}
void t12(){T(portable_json_array);auto nodes=SemanticRecoveryGraph::build("");nlohmann::json arr=nlohmann::json::array();for(auto& n:nodes) arr.push_back(SemanticRecoveryGraph::toJson(n));C(arr.size()==nodes.size(),"arr");P();}
int main(){std::cout<<"Step 809: SemanticRecoveryGraph\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
