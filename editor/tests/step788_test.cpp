// Step 788: Sprint 55 integration summary + regression (8 tests)
#include "Sprint55IntegrationSummary.h"
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(summary_constructable);auto r=Sprint55IntegrationSummary::run();C(r.stepStart==779&&r.stepEnd==788,"range");P();}
void t2(){T(adapters_ready);auto r=Sprint55IntegrationSummary::run();C(r.prologReady&&r.erlangReady&&r.elixirReady,"adapters");P();}
void t3(){T(policies_ready);auto r=Sprint55IntegrationSummary::run();C(r.logicPolicyReady&&r.actorPolicyReady,"policy");P();}
void t4(){T(supervision_gate_ready);auto r=Sprint55IntegrationSummary::run();C(r.supervisionReady&&r.gateReady,"gate");P();}
void t5(){T(report_ready);auto r=Sprint55IntegrationSummary::run();C(r.reportReady,"report");P();}
void t6(){T(mcp_tool_ready);auto r=Sprint55IntegrationSummary::run();C(r.mcpToolReady,"mcp");P();}
void t7(){T(files_sorted);auto r=Sprint55IntegrationSummary::run();C(std::is_sorted(r.filesAdded.begin(),r.filesAdded.end()),"sorted");P();}
void t8(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_ast_native_family")ok=true;C(ok,"prev");P();}
int main(){std::cout<<"Step 788: Sprint 55 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();auto r=Sprint55IntegrationSummary::run();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return (!r.success||f)?1:0;}
