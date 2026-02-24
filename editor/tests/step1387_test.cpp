#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(tool_reg); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}}); bool found=false; for(auto&t:r["result"]["tools"]) if(t["name"]=="whetstone_verify_requirements") found=true; C(found,"r"); P();}
void t2(){T(missing_fail); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",nlohmann::json::object()}}}}); auto j=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>()); C(!j["success"].get<bool>(),"f"); P();}
void t3(){T(success); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",{{"id","x1"}}}}}}); auto j=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>()); C(j["success"].get<bool>(),"s"); P();}
void t4(){T(has_data); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",{{"id","x1"}}}}}}); auto j=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>()); C(j.contains("data"),"d"); P();}
void t5(){T(id_echo); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",{{"id","x1"}}}}}}); auto j=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>()); C(j["id"]=="x1","i"); P();}
void t6(){T(tool_echo); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",{{"id","x1"}}}}}}); auto j=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>()); C(j["tool"]=="whetstone_verify_requirements","t"); P();}
void t7(){T(status_ok); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",{{"id","x1"}}}}}}); auto j=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>()); C(j["status"]=="ok","o"); P();}
void t8(){T(deterministic); MCPServer s; auto r1=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",{{"id","x1"}}}}}}); auto r2=s.handleRequest({{"jsonrpc","2.0"},{"id",2},{"method","tools/call"},{"params",{{"name","whetstone_verify_requirements"},{"arguments",{{"id","x1"}}}}}}); auto j1=nlohmann::json::parse(r1["result"]["content"][0]["text"].get<std::string>()); auto j2=nlohmann::json::parse(r2["result"]["content"][0]["text"].get<std::string>()); C(j1["data"].dump()==j2["data"].dump(),"d"); P();}
int main(){ std::cout<<"Step 1387: `whetstone_verify_requirements` MCP tool\n"; t1();t2();t3();t4();t5();t6();t7();t8(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
