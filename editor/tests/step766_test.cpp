// Step 766: whetstone_transpile_managed_family MCP tool (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static nlohmann::json call(MCPServer& m,const std::string& n,const nlohmann::json& a){auto r=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}});return nlohmann::json::parse(r["result"]["content"][0].value("text","{}"));}
void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_managed_family")ok=true;C(ok,"reg");P();}
void t2(){T(source_lang_required);MCPServer m;auto o=call(m,"whetstone_transpile_managed_family",{{"target_language","csharp"},{"source","x"}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_language_missing","err");P();}
void t3(){T(target_lang_required);MCPServer m;auto o=call(m,"whetstone_transpile_managed_family",{{"source_language","kotlin"},{"source","x"}});C(!o.value("success",true),"fail");C(o.value("error","")=="target_language_missing","err");P();}
void t4(){T(source_required);MCPServer m;auto o=call(m,"whetstone_transpile_managed_family",{{"source_language","kotlin"},{"target_language","csharp"}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_missing","err");P();}
void t5(){T(unsupported_source);MCPServer m;auto o=call(m,"whetstone_transpile_managed_family",{{"source_language","java"},{"target_language","csharp"},{"source","x"}});C(!o.value("success",true),"fail");C(o.value("error","")=="unsupported_source_language","err");P();}
void t6(){T(success_case);MCPServer m;auto o=call(m,"whetstone_transpile_managed_family",{{"source_language","kotlin"},{"target_language","csharp"},{"source","suspend fun go(x:String?)=x"}});C(o.value("success",false),"succ");C(o.contains("nullability_bridge")&&o.contains("async_bridge")&&o.contains("adt_bridge"),"bridges");P();}
void t7(){T(machine_readable);MCPServer m;auto o=call(m,"whetstone_transpile_managed_family",{{"source_language","fsharp"},{"target_language","vbnet"},{"source","type R = A | B"}});C(o["lowering"].is_object()&&o["raising"].is_object(),"obj");P();}
void t8(){T(deterministic);MCPServer m;auto a=call(m,"whetstone_transpile_managed_family",{{"source_language","vbnet"},{"target_language","csharp"},{"source","Select Case x\nEnd Select"}}).dump();auto b=call(m,"whetstone_transpile_managed_family",{{"source_language","vbnet"},{"target_language","csharp"},{"source","Select Case x\nEnd Select"}}).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 766: transpile_managed_family tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
