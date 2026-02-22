// Step 808: Sprint 57 integration summary (8 tests)
#include "Sprint57IntegrationSummary.h"
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(summary_constructable);auto r=Sprint57IntegrationSummary::run();C(r.stepStart==799&&r.stepEnd==808,"range");P();}
void t2(){T(core_modules_ready);auto r=Sprint57IntegrationSummary::run();C(r.cReady&&r.wasmReady&&r.x86Ready&&r.armReady,"modules");P();}
void t3(){T(layout_ready);auto r=Sprint57IntegrationSummary::run();C(r.layoutReady,"layout");P();}
void t4(){T(contract_ready);auto r=Sprint57IntegrationSummary::run();C(r.contractReady,"contract");P();}
void t5(){T(acceptance_ready);auto r=Sprint57IntegrationSummary::run();C(r.acceptanceReady,"accept");P();}
void t6(){T(mcp_tool_ready);auto r=Sprint57IntegrationSummary::run();C(r.mcpToolReady,"mcp");P();}
void t7(){T(files_sorted);auto r=Sprint57IntegrationSummary::run();C(std::is_sorted(r.filesAdded.begin(),r.filesAdded.end()),"sorted");P();}
void t8(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_query_family")ok=true;C(ok,"prev");P();}
int main(){std::cout<<"Step 808: Sprint 57 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();auto r=Sprint57IntegrationSummary::run();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return (!r.success||f)?1:0;}
