// Step 798: Sprint 56 integration summary + regression (8 tests)
#include "Sprint56IntegrationSummary.h"
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(summary_constructable);auto r=Sprint56IntegrationSummary::run();C(r.stepStart==789&&r.stepEnd==798,"range");P();}
void t2(){T(core_adapters_ready);auto r=Sprint56IntegrationSummary::run();C(r.canonicalReady&&r.postgresReady&&r.tsqlReady&&r.mysqlReady,"core");P();}
void t3(){T(transaction_equivalence_ready);auto r=Sprint56IntegrationSummary::run();C(r.transactionReady&&r.equivalenceReady,"txeq");P();}
void t4(){T(divergence_report_ready);auto r=Sprint56IntegrationSummary::run();C(r.divergenceReady&&r.reportReady,"div");P();}
void t5(){T(mcp_tool_ready);auto r=Sprint56IntegrationSummary::run();C(r.mcpToolReady,"mcp");P();}
void t6(){T(files_sorted);auto r=Sprint56IntegrationSummary::run();C(std::is_sorted(r.filesAdded.begin(),r.filesAdded.end()),"sorted");P();}
void t7(){T(success_true);auto r=Sprint56IntegrationSummary::run();C(r.success,"success");P();}
void t8(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_logic_actor_family")ok=true;C(ok,"prev");P();}
int main(){std::cout<<"Step 798: Sprint 56 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();auto r=Sprint56IntegrationSummary::run();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return (!r.success||f)?1:0;}
