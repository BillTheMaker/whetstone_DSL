// Step 885: whetstone_enqueue_pair_upgrade MCP tool (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_enqueue_pair_upgrade") found=true;
    C(found,"tool found");P();}

void t2(){T(no_pair_id_fails);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t3(){T(invalid_priority_fails);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","bogus"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t4(){T(valid_enqueue_succeeds);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","critical"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t5(){T(entry_id_in_response);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","revenue"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res.contains("entry_id"),"entry_id");P();}

void t6(){T(priority_class_in_response);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","critical"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["priority_class"]=="critical","priority");P();}

void t7(){T(queue_size_increments);
    MCPServer srv;
    srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","coverage"}}}}}});
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",2},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","rust->go"},{"priority","revenue"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["queue_size"].get<int>()==2,"size");P();}

void t8(){T(priority_level_in_response);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_enqueue_pair_upgrade"},{"arguments",{{"pair_id","py->cpp"},{"priority","critical"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["priority_level"].get<int>()==4,"level");P();}

int main(){
    std::cout<<"Step 885: whetstone_enqueue_pair_upgrade\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
