// Step 747: Family-level acceptance report generator (8 tests)
#include "systems/SystemsFamilyAcceptanceReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static std::vector<SystemPairCompatibility> sample(){return {{"c","cpp","stable"},{"go","rust","beta"}};}
void t1(){T(report_pair_count);auto r=SystemsFamilyAcceptanceReportModel::build(sample());C(r.pairCount==2,"pairs");P();}
void t2(){T(report_stable_count);auto r=SystemsFamilyAcceptanceReportModel::build(sample());C(r.stableCount==1,"stable");P();}
void t3(){T(report_beta_count);auto r=SystemsFamilyAcceptanceReportModel::build(sample());C(r.betaCount==1,"beta");P();}
void t4(){T(report_pairs_json);auto r=SystemsFamilyAcceptanceReportModel::build(sample());C(r.pairs.is_array(),"arr");P();}
void t5(){T(to_json_shape);auto j=SystemsFamilyAcceptanceReportModel::toJson(SystemsFamilyAcceptanceReportModel::build(sample()));C(j.contains("pair_count")&&j.contains("stable_count")&&j.contains("beta_count")&&j.contains("pairs"),"shape");P();}
void t6(){T(machine_readable);auto j=SystemsFamilyAcceptanceReportModel::toJson(SystemsFamilyAcceptanceReportModel::build(sample()));C(j.is_object(),"obj");P();}
void t7(){T(empty_supported);auto r=SystemsFamilyAcceptanceReportModel::build({});C(r.pairCount==0&&r.stableCount==0&&r.betaCount==0,"empty");P();}
void t8(){T(deterministic);auto a=SystemsFamilyAcceptanceReportModel::toJson(SystemsFamilyAcceptanceReportModel::build(sample())).dump();auto b=SystemsFamilyAcceptanceReportModel::toJson(SystemsFamilyAcceptanceReportModel::build(sample())).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 747: SystemsFamilyAcceptanceReport\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
