// Step 806: whetstone_transpile_low_level_family tool (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static nlohmann::json call(MCPServer& m,const std::string& n,const nlohmann::json& a){auto r=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}});return nlohmann::json::parse(r["result"]["content"][0].value("text","{}"));}
void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_low_level_family")ok=true;C(ok,"reg");P();}
void t2(){T(source_lang_required);MCPServer m;auto o=call(m,"whetstone_transpile_low_level_family",{{"source","mov"},{"target_language","c"}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_language_missing","err");P();}
void t3(){T(source_required);MCPServer m;auto o=call(m,"whetstone_transpile_low_level_family",{{"source_language","x86"},{"target_language","c"}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_missing","err");P();}
void t4(){T(target_required);MCPServer m;auto o=call(m,"whetstone_transpile_low_level_family",{{"source_language","x86"},{"source","mov"}});C(!o.value("success",true),"fail");C(o.value("error","")=="target_language_missing","err");P();}
void t5(){T(unsupported_source);MCPServer m;auto o=call(m,"whetstone_transpile_low_level_family",{{"source_language","java"},{"source","class"},{"target_language","c"}});C(!o.value("success",true),"fail");C(o.value("error","")=="unsupported_source_language","err");P();}
void t6(){T(success_case);MCPServer m;auto o=call(m,"whetstone_transpile_low_level_family",{{"source_language","x86"},{"source","mov"},{"target_language","arm"}});C(o.value("success",false),"succ");C(o.contains("layout")&&o.contains("contract"),"parts");P();}
void t7(){T(machine_readable);MCPServer m;auto o=call(m,"whetstone_transpile_low_level_family",{{"source_language","c"},{"source","int x;"},{"target_language","wasm"}});C(o["acceptance"].is_object(),"acc");P();}
void t8(){T(deterministic);MCPServer m;auto a=call(m,"whetstone_transpile_low_level_family",{{"source_language","wasm"},{"source","module"},{"target_language","c"}}).dump();auto b=call(m,"whetstone_transpile_low_level_family",{{"source_language","wasm"},{"source","module"},{"target_language","c"}}).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 806: transpile_low_level_family tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
