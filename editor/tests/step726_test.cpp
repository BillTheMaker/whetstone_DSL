// Step 726: whetstone_verify_executable_equivalence MCP tool (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static nlohmann::json call(MCPServer& m,const std::string& n,const nlohmann::json& a){auto r=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}});return nlohmann::json::parse(r["result"]["content"][0].value("text","{}"));}
static nlohmann::json vectors(){return nlohmann::json::array({{{"id","v1"},{"input",{{"x",1}}},{"expected","ok1"}},{{"id","v2"},{"input",{{"x",2}}},{"expected","ok2"}}});}

void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_verify_executable_equivalence")ok=true;C(ok,"reg");P();}
void t2(){T(vectors_required);MCPServer m;auto o=call(m,"whetstone_verify_executable_equivalence",nlohmann::json::object());C(!o.value("success",true),"fail");C(o.value("error","")=="vectors_missing","err");P();}
void t3(){T(success_output);MCPServer m;auto o=call(m,"whetstone_verify_executable_equivalence",{{"vectors",vectors()}});C(o.value("success",false),"succ");P();}
void t4(){T(equivalent_true_for_equal_outputs);MCPServer m;auto o=call(m,"whetstone_verify_executable_equivalence",{{"vectors",vectors()}});C(o.value("equivalent",false),"eq");P();}
void t5(){T(differential_present);MCPServer m;auto o=call(m,"whetstone_verify_executable_equivalence",{{"vectors",vectors()}});C(o.contains("differential"),"diff");P();}
void t6(){T(property_present);MCPServer m;auto o=call(m,"whetstone_verify_executable_equivalence",{{"vectors",vectors()},{"property_trials",4}});C(o.contains("property"),"prop");P();}
void t7(){T(evidence_present);MCPServer m;auto o=call(m,"whetstone_verify_executable_equivalence",{{"vectors",vectors()},{"fuzz_seeds",nlohmann::json::array({3,1})}});C(o.contains("evidence"),"ev");P();}
void t8(){T(deterministic_output);MCPServer m;auto a=call(m,"whetstone_verify_executable_equivalence",{{"vectors",vectors()},{"property_trials",5},{"fuzz_seeds",nlohmann::json::array({5,2,1})}}).dump();auto b=call(m,"whetstone_verify_executable_equivalence",{{"vectors",vectors()},{"property_trials",5},{"fuzz_seeds",nlohmann::json::array({5,2,1})}}).dump();C(a==b,"det");P();}

int main(){std::cout<<"Step 726: verify_executable_equivalence tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
