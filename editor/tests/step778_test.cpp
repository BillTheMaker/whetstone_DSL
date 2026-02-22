// Step 778: Sprint 54 integration summary + regression (8 tests)
#include "Sprint54IntegrationSummary.h"
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(summary_constructable);auto r=Sprint54IntegrationSummary::run();C(r.stepStart==769&&r.stepEnd==778,"range");P();}
void t2(){T(core_adapters_ready);auto r=Sprint54IntegrationSummary::run();C(r.canonicalReady&&r.lispReady&&r.schemeReady&&r.elispReady&&r.smalltalkReady,"core");P();}
void t3(){T(macro_eval_ready);auto r=Sprint54IntegrationSummary::run();C(r.macroReady&&r.evalReady,"risk");P();}
void t4(){T(benchmark_ready);auto r=Sprint54IntegrationSummary::run();C(r.benchmarkReady,"bench");P();}
void t5(){T(mcp_tool_ready);auto r=Sprint54IntegrationSummary::run();C(r.mcpToolReady,"mcp");P();}
void t6(){T(files_list_populated);auto r=Sprint54IntegrationSummary::run();C(!r.filesAdded.empty(),"files");P();}
void t7(){T(files_sorted);auto r=Sprint54IntegrationSummary::run();C(std::is_sorted(r.filesAdded.begin(),r.filesAdded.end()),"sorted");P();}
void t8(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_managed_family")ok=true;C(ok,"prev");P();}
int main(){std::cout<<"Step 778: Sprint 54 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();auto r=Sprint54IntegrationSummary::run();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return (!r.success||f)?1:0;}
