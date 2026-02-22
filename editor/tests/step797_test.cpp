// Step 797: Data family acceptance report (8 tests)
#include "data_query/DataFamilyAcceptanceReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static QueryDivergencePacket mk(const std::string& lvl){QueryDivergencePacket p; p.level=lvl; p.reasons={lvl}; return p;}
void t1(){T(total_count);auto r=DataFamilyAcceptanceReportModel::build({mk("low"),mk("high")});C(r.total==2,"total");P();}
void t2(){T(high_risk_count);auto r=DataFamilyAcceptanceReportModel::build({mk("high"),mk("high")});C(r.highRisk==2,"high");P();}
void t3(){T(medium_risk_count);auto r=DataFamilyAcceptanceReportModel::build({mk("medium"),mk("high")});C(r.mediumRisk==1,"med");P();}
void t4(){T(items_array);auto r=DataFamilyAcceptanceReportModel::build({mk("low")});C(r.items.is_array(),"items");P();}
void t5(){T(items_size);auto r=DataFamilyAcceptanceReportModel::build({mk("low"),mk("high")});C(r.items.size()==2,"items");P();}
void t6(){T(json_shape);auto j=DataFamilyAcceptanceReportModel::toJson(DataFamilyAcceptanceReportModel::build({mk("low")}));C(j.contains("total")&&j.contains("items"),"shape");P();}
void t7(){T(machine_readable);auto j=DataFamilyAcceptanceReportModel::toJson(DataFamilyAcceptanceReportModel::build({mk("low")}));C(j.is_object(),"obj");P();}
void t8(){T(deterministic);auto a=DataFamilyAcceptanceReportModel::toJson(DataFamilyAcceptanceReportModel::build({mk("low"),mk("high")})).dump();auto b=DataFamilyAcceptanceReportModel::toJson(DataFamilyAcceptanceReportModel::build({mk("low"),mk("high")})).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 797: DataFamilyAcceptanceReport\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
