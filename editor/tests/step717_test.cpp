// Step 717: whetstone_generate_cpp_from_ir MCP tool (8 tests)

#include "MCPServer.h"
#include "Sprint48IntegrationSummary.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static nlohmann::json callTool(MCPServer& m, const std::string& n, const nlohmann::json& a){
    nlohmann::json req={{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}};
    auto resp=m.handleRequest(req);
    return nlohmann::json::parse(resp["result"]["content"][0].value("text","{}"));
}

void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool found=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_generate_cpp_from_ir")found=true;C(found,"missing");P();}
void t2(){T(ir_required);MCPServer m;auto o=callTool(m,"whetstone_generate_cpp_from_ir",nlohmann::json::object());C(!o.value("success",true),"expected fail");C(o.value("error","")=="ir_missing","error");P();}
void t3(){T(profile_validated);MCPServer m;auto o=callTool(m,"whetstone_generate_cpp_from_ir",{{"ir",SemanticCoreIRModel::toJson(Sprint48IntegrationSummary::sampleIr())},{"profile","bad"}});C(!o.value("success",true),"expected fail");C(o.value("error","")=="profile_invalid","error");P();}
void t4(){T(valid_call_success);MCPServer m;auto o=callTool(m,"whetstone_generate_cpp_from_ir",{{"ir",SemanticCoreIRModel::toJson(Sprint48IntegrationSummary::sampleIr())},{"profile","safe-first"},{"projectName","demo"}});C(o.value("success",false),"expected success");P();}
void t5(){T(includes_files_packet);MCPServer m;auto o=callTool(m,"whetstone_generate_cpp_from_ir",{{"ir",SemanticCoreIRModel::toJson(Sprint48IntegrationSummary::sampleIr())}});C(o.contains("files"),"files missing");C(o["files"].contains("CMakeLists.txt"),"cmake missing");P();}
void t6(){T(includes_policy_packets);MCPServer m;auto o=callTool(m,"whetstone_generate_cpp_from_ir",{{"ir",SemanticCoreIRModel::toJson(Sprint48IntegrationSummary::sampleIr())}});C(o.contains("ownership")&&o.contains("borrowMapping")&&o.contains("templates"),"policy packets");P();}
void t7(){T(review_required_for_panic_gap);MCPServer m;auto o=callTool(m,"whetstone_generate_cpp_from_ir",{{"ir",SemanticCoreIRModel::toJson(Sprint48IntegrationSummary::sampleIr())}});C(o.value("reviewRequired",false),"review expected");P();}
void t8(){T(deterministic_output);MCPServer m;auto a=callTool(m,"whetstone_generate_cpp_from_ir",{{"ir",SemanticCoreIRModel::toJson(Sprint48IntegrationSummary::sampleIr())}}).dump();auto b=callTool(m,"whetstone_generate_cpp_from_ir",{{"ir",SemanticCoreIRModel::toJson(Sprint48IntegrationSummary::sampleIr())}}).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 717: Cpp generation MCP tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
