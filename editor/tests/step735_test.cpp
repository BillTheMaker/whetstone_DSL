// Step 735: whetstone_run_porting_gates MCP tool (8 tests)
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static nlohmann::json call(MCPServer& m,const std::string& n,const nlohmann::json& a){auto r=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/call"},{"params",{{"name",n},{"arguments",a}}}});return nlohmann::json::parse(r["result"]["content"][0].value("text","{}"));}
static nlohmann::json baseArgs(){return {{"security_findings",nlohmann::json::array()},
                                          {"sanitizer",{{"require_asan",true},{"require_ubsan",true},{"asan_clean",true},{"ubsan_clean",true}}},
                                          {"dependencies",nlohmann::json::array()},
                                          {"benchmarks",nlohmann::json::array({{{"name","b"},{"baseline_ms",10.0},{"target_ms",10.5}}})},
                                          {"thresholds",{{"max_high_severity_findings",0},{"max_perf_regression_pct",10.0}}}};}
void t1(){T(tool_registered);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_run_porting_gates")ok=true;C(ok,"reg");P();}
void t2(){T(success_with_defaults);MCPServer m;auto o=call(m,"whetstone_run_porting_gates",baseArgs());C(o.value("success",false),"succ");P();}
void t3(){T(gate_result_present);MCPServer m;auto o=call(m,"whetstone_run_porting_gates",baseArgs());C(o.contains("gate_result"),"gate");P();}
void t4(){T(pass_when_clean);MCPServer m;auto o=call(m,"whetstone_run_porting_gates",baseArgs());C(o["gate_result"].value("pass",false),"pass");P();}
void t5(){T(block_on_security_high);MCPServer m;auto a=baseArgs();a["security_findings"]=nlohmann::json::array({{{"id","x"},{"severity","high"},{"file","f.cpp"}}});auto o=call(m,"whetstone_run_porting_gates",a);C(!o["gate_result"].value("pass",true),"block");P();}
void t6(){T(severity_present);MCPServer m;auto o=call(m,"whetstone_run_porting_gates",baseArgs());C(o.contains("severity")&&o["severity"].is_object(),"sev");P();}
void t7(){T(machine_readable);MCPServer m;auto o=call(m,"whetstone_run_porting_gates",baseArgs());C(o["security"].is_object()&&o["sanitizer"].is_object()&&o["performance"].is_object(),"obj");P();}
void t8(){T(deterministic);MCPServer m;auto a=call(m,"whetstone_run_porting_gates",baseArgs()).dump();auto b=call(m,"whetstone_run_porting_gates",baseArgs()).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 735: run_porting_gates tool\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
