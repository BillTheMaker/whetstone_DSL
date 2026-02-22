// Step 856: RegisterFailureTelemetryTools - get_top_adapter_gaps (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_get_top_adapter_gaps") found=true;
    C(found,"tool found");P();}

void t2(){T(call_no_args);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_top_adapter_gaps"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["success"].get<bool>(),"success");P();}

void t3(){T(gaps_array);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_top_adapter_gaps"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["gaps"].is_array(),"gaps array");P();}

void t4(){T(total_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_top_adapter_gaps"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("total"),"total");P();}

void t5(){T(pair_id_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_top_adapter_gaps"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["gaps"][0].contains("pair_id"),"pair_id");P();}

void t6(){T(description_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_top_adapter_gaps"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["gaps"][0].contains("description"),"description");P();}

void t7(){T(priority_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_top_adapter_gaps"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["gaps"][0].contains("priority"),"priority");P();}

void t8(){T(with_top_n);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_top_adapter_gaps"},{"arguments",{{"top_n",1}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["total"].get<int>()==1,"total=1");P();}

int main(){
    std::cout<<"Step 856: get_top_adapter_gaps\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
