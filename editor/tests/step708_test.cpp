// Step 708: Sprint 47 integration summary + regression (8 tests)

#include "Sprint47IntegrationSummary.h"

#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(summary_constructable);auto r=Sprint47IntegrationSummary::run();C(r.stepStart==699,"construct");P();}
void t2(){T(all_modules_ready);auto r=Sprint47IntegrationSummary::run();C(r.ownershipReady&&r.lifetimeReady&&r.traitReady&&r.genericReady&&r.errorReady&&r.asyncReady&&r.macroReady&&r.unsafeReady,"modules not ready");P();}
void t3(){T(mcp_tool_listed);auto r=Sprint47IntegrationSummary::run();C(r.mcpToolReady,"mcp tool missing");P();}
void t4(){T(step_range_correct);auto r=Sprint47IntegrationSummary::run();C(r.stepStart==699&&r.stepEnd==708,"range wrong");P();}
void t5(){T(files_list_populated);auto r=Sprint47IntegrationSummary::run();C(!r.filesAdded.empty(),"files empty");P();}
void t6(){T(deterministic_files_order);auto a=Sprint47IntegrationSummary::run();auto b=Sprint47IntegrationSummary::run();C(a.filesAdded==b.filesAdded,"nondeterministic files");P();}
void t7(){T(regression_old_tool_still_present);MCPServer m;auto l=m.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});bool found=false;for(auto&t:l["result"]["tools"])if(t.value("name","")=="whetstone_get_language_matrix")found=true;C(found,"legacy tool missing");P();}
void t8(){T(success_true);auto r=Sprint47IntegrationSummary::run();C(r.success,"success false");P();}

int main(){std::cout<<"Step 708: Sprint 47 integration summary\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
