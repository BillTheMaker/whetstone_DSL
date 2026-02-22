// Step 846: RegisterCertificationTools - get_certification_status (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_get_certification_status") found=true;
    C(found,"tool found");P();}

void t2(){T(call_no_args);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_certification_status"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["success"].get<bool>(),"success");P();}

void t3(){T(call_with_pair_id);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_certification_status"},{"arguments",{{"pair_id","py->cpp"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["success"].get<bool>(),"success");P();}

void t4(){T(entries_array);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_certification_status"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["entries"].is_array(),"entries array");P();}

void t5(){T(total_count);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_certification_status"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("total"),"total");P();}

void t6(){T(pass_rate_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_certification_status"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(!res["entries"].empty()&&res["entries"][0].contains("pass_rate"),"pass_rate");P();}

void t7(){T(status_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_certification_status"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["entries"][0].contains("status"),"status field");P();}

void t8(){T(json_structure);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_certification_status"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("success")&&res.contains("entries")&&res.contains("total"),"keys");P();}

int main(){
    std::cout<<"Step 846: get_certification_status\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
