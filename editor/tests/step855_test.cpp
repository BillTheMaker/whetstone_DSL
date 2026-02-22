// Step 855: RegisterFailureTelemetryTools - get_failure_trends (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_get_failure_trends") found=true;
    C(found,"tool found");P();}

void t2(){T(call_no_args);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_failure_trends"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["success"].get<bool>(),"success");P();}

void t3(){T(trends_array);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_failure_trends"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["trends"].is_array(),"trends array");P();}

void t4(){T(total_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_failure_trends"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("total"),"total");P();}

void t5(){T(count_field_in_trends);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_failure_trends"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["trends"][0].contains("count"),"count field");P();}

void t6(){T(severity_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_failure_trends"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["trends"][0].contains("severity"),"severity");P();}

void t7(){T(with_limit);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_failure_trends"},{"arguments",{{"limit",1}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["total"].get<int>()<=3,"within limit");P();}

void t8(){T(code_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_failure_trends"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["trends"][0].contains("code"),"code field");P();}

int main(){
    std::cout<<"Step 855: get_failure_trends\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
