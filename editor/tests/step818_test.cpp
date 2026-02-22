// Step 818: Sprint 58 integration summary (8 tests)
#include "Sprint58IntegrationSummary.h"
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(summary_constructable);auto r=Sprint58IntegrationSummary::run();C(r.stepStart==809&&r.stepEnd==818,"range");P();}
void t2(){T(core_ready);auto r=Sprint58IntegrationSummary::run();C(r.graphReady&&r.apiReady&&r.assumptionReady,"core");P();}
void t3(){T(ambiguity_ready);auto r=Sprint58IntegrationSummary::run();C(r.ambiguityReady&&r.reviewReady,"amb");P();}
void t4(){T(confidence_readiness);auto r=Sprint58IntegrationSummary::run();std::cout<<"conf="<<r.confidenceReady<<" read="<<r.readinessReady<<" ready="<<r.success<<"\n";C(r.confidenceReady&&r.readinessReady,"score");P();}
void t5(){T(mcp_tool_ready);auto r=Sprint58IntegrationSummary::run();C(r.mcpToolReady,"mcp");P();}
void t6(){T(files_sorted);auto r=Sprint58IntegrationSummary::run();C(std::is_sorted(r.filesAdded.begin(),r.filesAdded.end()),"sorted");P();}
void t7(){T(success_true);auto r=Sprint58IntegrationSummary::run();C(r.success,"success");P();}
void t8(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_ingest_legacy_to_ir")ok=true;C(ok,"prev");P();}
int main(){std::cout<<"Step 818: Sprint 58 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();auto r=Sprint58IntegrationSummary::run();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return (!r.success||f)?1:0;}
