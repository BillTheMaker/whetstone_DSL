// Step 816: whetstone_ingest_legacy_to_ir tool (8 tests)
#include "MCPServer.h"
#include <nlohmann/json.hpp>
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static nlohmann::json call(MCPServer& m,const std::string& n,const nlohmann::json& a){auto r=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}});return nlohmann::json::parse(r["result"]["content"][0].value("text","{}"));}
void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_ingest_legacy_to_ir")ok=true;C(ok,"reg");P();}
void t2(){T(source_required);MCPServer m;auto o=call(m,"whetstone_ingest_legacy_to_ir",{{"files",nlohmann::json::array({"a"})}});C(!o.value("success",true),"fail");C(o.value("error","")=="source_missing","err");P();}
void t3(){T(success_case);MCPServer m;auto o=call(m,"whetstone_ingest_legacy_to_ir",nlohmann::json{
    {"source","TODO"},
    {"api","read"},
    {"files", nlohmann::json::array({"a"})}
});C(o.value("success",false),"succ");P();}
void t4(){T(graph_present);MCPServer m;auto o=call(m,"whetstone_ingest_legacy_to_ir",{{"source","code"}});C(o.contains("graph"),"graph");P();}
void t5(){T(intent_present);MCPServer m;auto o=call(m,"whetstone_ingest_legacy_to_ir",{{"source","code"}});C(o.contains("api_intent"),"intent");P();}
void t6(){T(review_queue);MCPServer m;auto o=call(m,"whetstone_ingest_legacy_to_ir",{{"source","TODO"}});C(o.contains("review_queue"),"review");P();}
void t7(){T(acceptance_ready);MCPServer m;auto o=call(m,"whetstone_ingest_legacy_to_ir",{{"source","code"}});C(o.contains("readiness"),"ready");P();}
void t8(){T(deterministic);MCPServer m;auto a=call(m,"whetstone_ingest_legacy_to_ir",{{"source","code"}}).dump();auto b=call(m,"whetstone_ingest_legacy_to_ir",{{"source","code"}}).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 816: ingest_legacy_to_ir tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
