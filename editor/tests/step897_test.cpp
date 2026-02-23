// Step 897: whetstone_get_runtime_assumptions + whetstone_set_runtime_profile MCP tools (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(get_assumptions_registered);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}});
    bool found=false;
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_get_runtime_assumptions") found=true;
    C(found,"tool found");P();}

void t2(){T(set_profile_registered);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}});
    bool found=false;
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_set_runtime_profile") found=true;
    C(found,"tool found");P();}

void t3(){T(get_assumptions_empty_patterns);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_runtime_assumptions"},{"arguments",{{"runtime_id","jvm11"},{"patterns",nlohmann::json::array()}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>()&&res["count"]==0,"empty");P();}

void t4(){T(get_assumptions_returns_hints);
    MCPServer srv;
    nlohmann::json pats=nlohmann::json::array({"import java.util"});
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_runtime_assumptions"},{"arguments",{{"runtime_id","jvm11"},{"patterns",pats}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["count"].get<int>()==1,"one hint");P();}

void t5(){T(get_assumptions_missing_args_fail);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_runtime_assumptions"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t6(){T(set_profile_succeeds);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_runtime_profile"},{"arguments",{{"pair_id","py->cpp"},{"runtime_id","cpython3"},{"version","3.11"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t7(){T(set_profile_missing_pair_id_fail);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_runtime_profile"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t8(){T(set_profile_pair_id_in_response);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_set_runtime_profile"},{"arguments",{{"pair_id","py->cpp"},{"runtime_id","cpython3"},{"version","3.11"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["pair_id"]=="py->cpp","pair_id");P();}

int main(){
    std::cout<<"Step 897: Runtime pack MCP tools\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
