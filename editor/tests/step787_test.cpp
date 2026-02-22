// Step 787: Family acceptance and divergence report (8 tests)
#include "logic_actor/LogicActorAcceptanceReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static SemanticGatePacket gate(bool blocked,bool review){SemanticGatePacket g;g.blocked=blocked;g.reviewRequired=review;g.reasons={blocked?"blocked":"ok"};return g;}
void t1(){T(total_count);auto r=LogicActorAcceptanceReportModel::build({gate(false,false),gate(true,true)});C(r.total==2,"total");P();}
void t2(){T(blocked_count);auto r=LogicActorAcceptanceReportModel::build({gate(false,false),gate(true,true)});C(r.blocked==1,"blocked");P();}
void t3(){T(review_count);auto r=LogicActorAcceptanceReportModel::build({gate(false,false),gate(true,true)});C(r.reviewRequired==1,"review");P();}
void t4(){T(items_array);auto r=LogicActorAcceptanceReportModel::build({gate(false,false)});C(r.items.is_array(),"items");P();}
void t5(){T(items_len);auto r=LogicActorAcceptanceReportModel::build({gate(false,false),gate(true,true)});C(r.items.size()==2,"items");P();}
void t6(){T(json_shape);auto j=LogicActorAcceptanceReportModel::toJson(LogicActorAcceptanceReportModel::build({gate(false,false)}));C(j.contains("total")&&j.contains("items"),"shape");P();}
void t7(){T(machine_readable);auto j=LogicActorAcceptanceReportModel::toJson(LogicActorAcceptanceReportModel::build({gate(false,false)}));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=LogicActorAcceptanceReportModel::toJson(LogicActorAcceptanceReportModel::build({gate(false,false),gate(true,true)})).dump();auto b=LogicActorAcceptanceReportModel::toJson(LogicActorAcceptanceReportModel::build({gate(false,false),gate(true,true)})).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 787: LogicActorAcceptanceReport\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
