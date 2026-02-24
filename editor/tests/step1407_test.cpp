#include "Sprint117IntegrationSummary.h"
#include "MCPServer.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(verify); C(Sprint117IntegrationSummary::verify(),"v"); P();}
void t2(){T(sprint); auto j=Sprint117IntegrationSummary::toJson(); C(j["sprint"]==117,"s"); P();}
void t3(){T(steps); auto j=Sprint117IntegrationSummary::toJson(); C(j["steps"]==9,"s"); P();}
void t4(){T(theme); auto j=Sprint117IntegrationSummary::toJson(); C(j.contains("theme"),"t"); P();}
void t5(){T(tools); auto j=Sprint117IntegrationSummary::toJson(); C(j.contains("tools_added"),"t"); P();}
void t6(){T(reg_tool_1); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}}); bool found=false; for(auto&t:r["result"]["tools"]) if(t["name"]=="whetstone_configure_hivemind") found=true; C(found,"r"); P();}
void t7(){T(reg_tool_2); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}}); bool found=false; for(auto&t:r["result"]["tools"]) if(t["name"]=="whetstone_submit_iteration_job") found=true; C(found,"r"); P();}
void t8(){T(reg_tool_3); MCPServer s; auto r=s.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"},{"params",{}}}); bool found=false; for(auto&t:r["result"]["tools"]) if(t["name"]=="whetstone_poll_iteration_job") found=true; C(found,"r"); P();}
int main(){ std::cout<<"Step 1407: Sprint 117 integration summary + full-program regression\n"; t1();t2();t3();t4();t5();t6();t7();t8(); std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n"; return f?1:0; }
