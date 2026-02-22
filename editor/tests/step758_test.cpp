// Step 758: Sprint 52 integration summary + regression (8 tests)
#include "Sprint52IntegrationSummary.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(summary_constructable);auto r=Sprint52IntegrationSummary::run();C(r.stepStart==749,"construct");P();}
void t2(){T(core_modules_ready);auto r=Sprint52IntegrationSummary::run();C(r.pythonReady&&r.jsReady&&r.tsReady&&r.rubyReady&&r.luaReady&&r.policyReady&&r.riskReady&&r.reportReady,"core");P();}
void t3(){T(mcp_tool_ready);auto r=Sprint52IntegrationSummary::run();C(r.mcpToolReady,"mcp");P();}
void t4(){T(step_range_correct);auto r=Sprint52IntegrationSummary::run();C(r.stepStart==749&&r.stepEnd==758,"range");P();}
void t5(){T(files_list_populated);auto r=Sprint52IntegrationSummary::run();C(!r.filesAdded.empty(),"files");P();}
void t6(){T(deterministic_files_order);auto a=Sprint52IntegrationSummary::run();auto b=Sprint52IntegrationSummary::run();C(a.filesAdded==b.filesAdded,"order");P();}
void t7(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool found=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_systems_family")found=true;C(found,"prior");P();}
void t8(){T(success_true);auto r=Sprint52IntegrationSummary::run();C(r.success,"success");P();}
int main(){std::cout<<"Step 758: Sprint 52 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
