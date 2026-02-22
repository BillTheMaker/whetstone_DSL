// Step 845: RegisterCertificationTools - run_certification_cycle (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_run_certification_cycle") found=true;
    C(found,"tool found");P();}

void t2(){T(call_with_cycle_id);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_run_certification_cycle"},{"arguments",{{"cycle_id","CYC-1"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["success"].get<bool>(),"success");P();}

void t3(){T(missing_cycle_id_fails);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_run_certification_cycle"},{"arguments",{}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(!res["success"].get<bool>(),"fails");P();}

void t4(){T(with_pairs_array);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_run_certification_cycle"},{"arguments",{{"cycle_id","CYC-1"},{"pairs",{"py->cpp","rust->cpp"}}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["pairs_tested"].get<int>()==2,"pairs_tested=2");P();}

void t5(){T(strategy_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_run_certification_cycle"},{"arguments",{{"cycle_id","CYC-1"},{"strategy","full_sweep"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["success"].get<bool>(),"success");P();}

void t6(){T(passed_count);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_run_certification_cycle"},{"arguments",{{"cycle_id","CYC-1"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("passed"),"passed field");P();}

void t7(){T(status_present);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_run_certification_cycle"},{"arguments",{{"cycle_id","CYC-1"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res["status"]=="complete","status=complete");P();}

void t8(){T(json_structure);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_run_certification_cycle"},{"arguments",{{"cycle_id","CYC-1"}}}}}});
    auto text=r["result"]["content"][0]["text"].get<std::string>();
    auto res=nlohmann::json::parse(text);
    C(res.contains("cycle_id")&&res.contains("pairs_tested"),"keys");P();}

int main(){
    std::cout<<"Step 845: RegisterCertificationTools (run_certification_cycle)\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
