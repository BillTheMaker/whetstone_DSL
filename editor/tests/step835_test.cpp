// Step 835: RegisterGraduationTools MCP test (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_get_transpile_support_matrix") found=true;
    C(found,"tool found");P();}

void t2(){T(call_no_args);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_transpile_support_matrix"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["success"].get<bool>(),"success");P();}

void t3(){T(call_with_tier_stable);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_transpile_support_matrix"},{"arguments",{{"tier","stable"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["total"].get<int>()==2,"2 stable pairs");P();}

void t4(){T(call_with_tier_beta);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_transpile_support_matrix"},{"arguments",{{"tier","beta"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["total"].get<int>()==2,"2 beta pairs");P();}

void t5(){T(call_with_tier_experimental);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_transpile_support_matrix"},{"arguments",{{"tier","experimental"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["total"].get<int>()==1,"1 experimental pair");P();}

void t6(){T(total_count_all);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_transpile_support_matrix"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["total"].get<int>()==5,"total=5");P();}

void t7(){T(pairs_array_present);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_transpile_support_matrix"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("pairs")&&res["pairs"].is_array(),"pairs array");P();}

void t8(){T(json_structure);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_transpile_support_matrix"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("success")&&res.contains("pairs")&&res.contains("total"),"keys");P();}

int main(){
    std::cout<<"Step 835: RegisterGraduationTools\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
