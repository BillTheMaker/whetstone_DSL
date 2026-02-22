// Step 718: Sprint 48 integration summary + regression (8 tests)

#include "Sprint48IntegrationSummary.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(summary_constructable);auto r=Sprint48IntegrationSummary::run();C(r.stepStart==709,"construct");P();}
void t2(){T(all_modules_ready);auto r=Sprint48IntegrationSummary::run();C(r.ownershipReady&&r.borrowReady&&r.traitReady&&r.templateReady&&r.errorReady&&r.asyncReady&&r.algorithmReady&&r.buildReady,"not ready");P();}
void t3(){T(mcp_tool_ready);auto r=Sprint48IntegrationSummary::run();C(r.mcpToolReady,"mcp missing");P();}
void t4(){T(step_range_correct);auto r=Sprint48IntegrationSummary::run();C(r.stepStart==709&&r.stepEnd==718,"range");P();}
void t5(){T(files_list_populated);auto r=Sprint48IntegrationSummary::run();C(!r.filesAdded.empty(),"files");P();}
void t6(){T(deterministic_files_order);auto a=Sprint48IntegrationSummary::run();auto b=Sprint48IntegrationSummary::run();C(a.filesAdded==b.filesAdded,"order");P();}
void t7(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool found=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_analyze_rust_semantics")found=true;C(found,"prior tool missing");P();}
void t8(){T(success_true);auto r=Sprint48IntegrationSummary::run();C(r.success,"success");P();}

int main(){std::cout<<"Step 718: Sprint 48 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
