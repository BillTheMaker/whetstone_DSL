// Step 707: whetstone_analyze_rust_semantics MCP tool (8 tests)

#include "MCPServer.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static nlohmann::json callTool(MCPServer& mcp, const std::string& name, const nlohmann::json& args) {
    nlohmann::json req = {{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",name},{"arguments",args}}}};
    auto resp = mcp.handleRequest(req);
    std::string text = resp["result"]["content"][0].value("text", "{}");
    return nlohmann::json::parse(text);
}

static const char* src =
    "trait Draw { fn draw(&self); }\n"
    "impl Draw for Canvas { fn draw(&self) {} }\n"
    "async fn run<'a, T>(x: &'a T) -> Result<Option<i32>, String> {\n"
    "  println!(\"hi\");\n"
    "  io().await;\n"
    "  panic!(\"x\");\n"
    "}\n"
    "unsafe { let p: *const i32 = core::ptr::null(); }\n";

void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool found=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_analyze_rust_semantics")found=true;C(found,"tool missing");P();}
void t2(){T(missing_source_errors);MCPServer m;auto out=callTool(m,"whetstone_analyze_rust_semantics",nlohmann::json::object());C(!out.value("success",true),"expected fail");C(out.value("error","")=="source_missing","wrong error");P();}
void t3(){T(valid_analysis_success);MCPServer m;auto out=callTool(m,"whetstone_analyze_rust_semantics",{{"source",src}});C(out.value("success",false),"expected success");P();}
void t4(){T(includes_all_packets);MCPServer m;auto out=callTool(m,"whetstone_analyze_rust_semantics",{{"source",src}});C(out.contains("ownership")&&out.contains("lifetimes")&&out.contains("traits")&&out.contains("generics")&&out.contains("errorModel")&&out.contains("asyncModel")&&out.contains("macroBoundaries")&&out.contains("unsafeRisk"),"missing packets");P();}
void t5(){T(review_required_when_unsafe);MCPServer m;auto out=callTool(m,"whetstone_analyze_rust_semantics",{{"source",src}});C(out.value("reviewRequired",false),"review should be true");P();}
void t6(){T(strict_unknown_macro_fails);MCPServer m;auto out=callTool(m,"whetstone_analyze_rust_semantics",{{"source","custom_macro!(x);"},{"strict",true}});C(!out.value("success",true),"expected strict fail");C(out.value("error","")=="unsupported_macro_boundary","wrong error");P();}
void t7(){T(machine_readable_output);MCPServer m;auto out=callTool(m,"whetstone_analyze_rust_semantics",{{"source",src}});C(out["unsafeRisk"].is_object(),"unsafeRisk not object");P();}
void t8(){T(deterministic_output);MCPServer m;auto a=callTool(m,"whetstone_analyze_rust_semantics",{{"source",src}}).dump();auto b=callTool(m,"whetstone_analyze_rust_semantics",{{"source",src}}).dump();C(a==b,"nondeterministic");P();}

int main(){std::cout<<"Step 707: Rust semantics MCP tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
