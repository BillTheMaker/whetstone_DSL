// Step 796: whetstone_transpile_query_family MCP tool (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static nlohmann::json call(MCPServer& m,const std::string& n,const nlohmann::json& a){auto r=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}});return nlohmann::json::parse(r["result"]["content"][0].value("text","{}"));}
void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_query_family")ok=true;C(ok,"reg");P();}
void t2(){T(source_dialect_required);MCPServer m;auto o=call(m,"whetstone_transpile_query_family",{{"target_dialect","mysql"},{"query","SELECT 1"}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_dialect_missing","err");P();}
void t3(){T(target_dialect_required);MCPServer m;auto o=call(m,"whetstone_transpile_query_family",{{"source_dialect","postgresql"},{"query","SELECT 1"}});C(!o.value("success",true),"fail");C(o.value("error","")=="target_dialect_missing","err");P();}
void t4(){T(query_required);MCPServer m;auto o=call(m,"whetstone_transpile_query_family",{{"source_dialect","postgresql"},{"target_dialect","mysql"}});C(!o.value("success",true),"fail");C(o.value("error","")=="query_missing","err");P();}
void t5(){T(unsupported_source);MCPServer m;auto o=call(m,"whetstone_transpile_query_family",{{"source_dialect","sqlite"},{"target_dialect","mysql"},{"query","SELECT 1"}});C(!o.value("success",true),"fail");C(o.value("error","")=="unsupported_source_dialect","err");P();}
void t6(){T(success_case);MCPServer m;auto o=call(m,"whetstone_transpile_query_family",{{"source_dialect","postgresql"},{"target_dialect","mysql"},{"query","SELECT COUNT(*) FROM a JOIN b ON a.id=b.id"}});C(o.value("success",false),"succ");C(o.contains("transaction")&&o.contains("equivalence")&&o.contains("divergence"),"parts");P();}
void t7(){T(machine_readable);MCPServer m;auto o=call(m,"whetstone_transpile_query_family",{{"source_dialect","tsql"},{"target_dialect","postgresql"},{"query","SELECT 1"}});C(o["lowering"].is_object()&&o["acceptance"].is_object(),"obj");P();}
void t8(){T(deterministic);MCPServer m;auto a=call(m,"whetstone_transpile_query_family",{{"source_dialect","mysql"},{"target_dialect","postgresql"},{"query","SELECT 1"}}).dump();auto b=call(m,"whetstone_transpile_query_family",{{"source_dialect","mysql"},{"target_dialect","postgresql"},{"query","SELECT 1"}}).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 796: transpile_query_family tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
