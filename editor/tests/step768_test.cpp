// Step 768: Sprint 53 integration summary + regression (8 tests)
#include "Sprint53IntegrationSummary.h"
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(summary_constructable);auto r=Sprint53IntegrationSummary::run();C(r.stepStart==759&&r.stepEnd==768,"range");P();}
void t2(){T(core_modules_ready);auto r=Sprint53IntegrationSummary::run();C(r.kotlinReady&&r.csharpReady&&r.fsharpReady&&r.vbnetReady,"mods");P();}
void t3(){T(bridges_ready);auto r=Sprint53IntegrationSummary::run();C(r.nullabilityReady&&r.asyncReady&&r.adtReady,"bridges");P();}
void t4(){T(promotion_ready);auto r=Sprint53IntegrationSummary::run();C(r.promotionReady,"promo");P();}
void t5(){T(mcp_tool_ready);auto r=Sprint53IntegrationSummary::run();C(r.mcpToolReady,"mcp");P();}
void t6(){T(files_list_populated);auto r=Sprint53IntegrationSummary::run();C(!r.filesAdded.empty(),"files");P();}
void t7(){T(files_sorted);auto r=Sprint53IntegrationSummary::run();C(std::is_sorted(r.filesAdded.begin(), r.filesAdded.end()),"sorted");P();}
void t8(){T(regression_prior_tool_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool ok=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_transpile_dynamic_family")ok=true;C(ok,"prev");P();}
int main(){std::cout<<"Step 768: Sprint 53 integration\n";t1();t2();t3();t4();t5();t6();t7();t8();auto r=Sprint53IntegrationSummary::run();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return (!r.success||f)?1:0;}
