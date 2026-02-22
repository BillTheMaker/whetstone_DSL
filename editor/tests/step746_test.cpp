// Step 746: whetstone_transpile_systems_family MCP tool (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static nlohmann::json call(MCPServer& m,const std::string& n,const nlohmann::json& a){auto r=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}});return nlohmann::json::parse(r["result"]["content"][0].value("text","{}"));}
void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_systems_family")ok=true;C(ok,"reg");P();}
void t2(){T(source_required);MCPServer m;auto o=call(m,"whetstone_transpile_systems_family",{{"target_language","c"},{"source","x"}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_language_missing","err");P();}
void t3(){T(target_required);MCPServer m;auto o=call(m,"whetstone_transpile_systems_family",{{"source_language","c"},{"source","x"}});C(!o.value("success",true),"fail");C(o.value("error","")=="target_language_missing","err");P();}
void t4(){T(source_code_required);MCPServer m;auto o=call(m,"whetstone_transpile_systems_family",{{"source_language","c"},{"target_language","go"}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_missing","err");P();}
void t5(){T(success_path);MCPServer m;auto o=call(m,"whetstone_transpile_systems_family",{{"source_language","c"},{"target_language","go"},{"source","int main(){}"}});C(o.value("success",false),"succ");P();}
void t6(){T(lowering_and_raising_present);MCPServer m;auto o=call(m,"whetstone_transpile_systems_family",{{"source_language","go"},{"target_language","java"},{"source","package main"}});C(o.contains("lowering")&&o.contains("raising"),"present");P();}
void t7(){T(pair_support_flag);MCPServer m;auto o=call(m,"whetstone_transpile_systems_family",{{"source_language","c"},{"target_language","go"},{"source","x"}});C(o.contains("supported_pair"),"support");P();}
void t8(){T(deterministic);MCPServer m;auto a=call(m,"whetstone_transpile_systems_family",{{"source_language","java"},{"target_language","go"},{"source","class A{}"},{"profile","safe-first"}}).dump();auto b=call(m,"whetstone_transpile_systems_family",{{"source_language","java"},{"target_language","go"},{"source","class A{}"},{"profile","safe-first"}}).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 746: transpile_systems_family tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
