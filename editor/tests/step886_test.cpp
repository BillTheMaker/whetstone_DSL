// Step 886: whetstone_get_upgrade_queue MCP tool (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_get_upgrade_queue") found=true;
    C(found,"tool found");P();}

void t2(){T(empty_queue_returns_success);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_upgrade_queue"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t3(){T(entries_array_present);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_upgrade_queue"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["entries"].is_array(),"entries");P();}

void t4(){T(total_active_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_upgrade_queue"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res.contains("total_active"),"total");P();}

void t5(){T(enqueue_then_get);
    MCPServer srv;
    srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","critical"}}}}}});
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",2},{"method","tools/call"},{"params",{{"name","whetstone_get_upgrade_queue"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["total_active"].get<int>()==1,"one");P();}

void t6(){T(sorted_by_priority);
    MCPServer srv;
    srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","experimental"}}}}}});
    srv.handleRequest({{"jsonrpc","2.0"},{"id",2},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","rust->go"},{"priority","critical"}}}}}});
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",3},{"method","tools/call"},{"params",{{"name","whetstone_get_upgrade_queue"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["entries"][0]["priority"]=="critical","sorted");P();}

void t7(){T(shown_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_upgrade_queue"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res.contains("shown"),"shown");P();}

void t8(){T(limit_respected);
    MCPServer srv;
    for(int i=0;i<3;++i){
        std::string pid="pair"+std::to_string(i);
        srv.handleRequest({{"jsonrpc","2.0"},{"id",i},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id",pid},{"priority","coverage"}}}}}});
    }
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",10},{"method","tools/call"},{"params",{{"name","whetstone_get_upgrade_queue"},{"arguments",{{"limit",2}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["shown"].get<int>()==2,"limit");P();}

int main(){
    std::cout<<"Step 886: whetstone_get_upgrade_queue\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
