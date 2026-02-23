// Step 865: whetstone_get_adapter_hints MCP tool (8 tests)
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
    for(auto& t:r["result"]["tools"]) if(t["name"]=="whetstone_get_adapter_hints") found=true;
    C(found,"tool found");P();}

void t2(){T(no_pair_id_fails);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_adapter_hints"},{"arguments",{}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(!res["success"].get<bool>(),"fail");P();}

void t3(){T(valid_pair_succeeds);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_adapter_hints"},{"arguments",{{"pair_id","py->cpp"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["success"].get<bool>(),"success");P();}

void t4(){T(model_available_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_adapter_hints"},{"arguments",{{"pair_id","py->cpp"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res.contains("model_available"),"field");P();}

void t5(){T(hints_array_present);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_adapter_hints"},{"arguments",{{"pair_id","py->cpp"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["hints"].is_array(),"hints");P();}

void t6(){T(hint_count_field);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_adapter_hints"},{"arguments",{{"pair_id","py->cpp"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res.contains("hint_count"),"count");P();}

void t7(){T(with_features);
    MCPServer srv;
    nlohmann::json feats=nlohmann::json::array(); feats.push_back("f1"); feats.push_back("f2");
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_adapter_hints"},{"arguments",{{"pair_id","py->cpp"},{"features",feats}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["hint_count"].get<int>()==2,"count 2");P();}

void t8(){T(pair_id_in_response);
    MCPServer srv;
    auto r=srv.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name","whetstone_get_adapter_hints"},{"arguments",{{"pair_id","rust->go"}}}}}});
    auto res=nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
    C(res["pair_id"]=="rust->go","pair_id");P();}

int main(){
    std::cout<<"Step 865: whetstone_get_adapter_hints\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
