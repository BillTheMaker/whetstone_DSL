// Step 907: whetstone_plan_migration_path + whetstone_track_migration_progress MCP tools (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(plan_migration_registered);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}});
    bool found=false;
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_plan_migration_path") found=true;
    C(found,"tool found");P();}

void t2(){T(track_migration_registered);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}});
    bool found=false;
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_track_migration_progress") found=true;
    C(found,"tool found");P();}

void t3(){T(plan_migration_missing_args_fail);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_plan_migration_path"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t4(){T(plan_migration_returns_steps);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_plan_migration_path"},{"arguments",{{"pair_id","py->cpp"},{"source_runtime","cpython3"},{"target_runtime","gcc"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>()&&res.contains("steps"),"steps");P();}

void t5(){T(plan_migration_has_feasibility);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_plan_migration_path"},{"arguments",{{"pair_id","py->cpp"},{"source_runtime","cpython3"},{"target_runtime","gcc"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res.contains("feasibility"),"feas");P();}

void t6(){T(track_progress_missing_args_fail);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_track_migration_progress"},{"arguments",nlohmann::json::object()}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t7(){T(track_progress_starts_tracking);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_track_migration_progress"},{"arguments",{{"pair_id","py->cpp"},{"action","start"},{"total_steps",5}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t8(){T(track_progress_has_status);
    MCPServer srv;
    srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_track_migration_progress"},{"arguments",{{"pair_id","py->cpp"},{"action","start"},{"total_steps",5}}}}}});
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",2},{"method","tools/call"},{"params",{{"name","whetstone_track_migration_progress"},{"arguments",{{"pair_id","py->cpp"},{"action","get"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res.contains("status"),"status");P();}

int main(){
    std::cout<<"Step 907: Migration planning MCP tools\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
