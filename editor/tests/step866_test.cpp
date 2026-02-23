// Step 866: whetstone_set_hint_policy MCP tool (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(tool_registered);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}});
    bool found=false;
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_set_hint_policy") found=true;
    C(found,"tool found");P();}

void t2(){T(no_pair_id_fails);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_hint_policy"},{"arguments",{{"policy","enabled"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t3(){T(invalid_policy_fails);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_hint_policy"},{"arguments",{{"pair_id","py->cpp"},{"policy","invalid"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t4(){T(enabled_policy_succeeds);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_hint_policy"},{"arguments",{{"pair_id","py->cpp"},{"policy","enabled"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t5(){T(disabled_policy_succeeds);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_hint_policy"},{"arguments",{{"pair_id","py->cpp"},{"policy","disabled"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t6(){T(audit_only_policy_succeeds);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_hint_policy"},{"arguments",{{"pair_id","py->cpp"},{"policy","audit_only"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t7(){T(response_has_policy);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_hint_policy"},{"arguments",{{"pair_id","py->cpp"},{"policy","enabled"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["policy"]=="enabled","policy");P();}

void t8(){T(response_has_pair_id);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_hint_policy"},{"arguments",{{"pair_id","rust->go"},{"policy","disabled"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["pair_id"]=="rust->go","pair_id");P();}

int main(){
    std::cout<<"Step 866: whetstone_set_hint_policy\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
