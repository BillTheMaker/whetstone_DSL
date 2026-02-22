// Step 807: Low-level acceptance report (8 tests)
#include "low_level/LowLevelAcceptanceReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
static LowLevelEntry mk(bool review,bool waived,std::string reason){LowLevelEntry e; e.reviewRequired=review; e.waived=waived; e.reason=reason; return e;}
void t1(){T(total_count);auto r=LowLevelAcceptanceReportModel::build({mk(true,false,"a")});C(r.total==1,"total");P();}
void t2(){T(waiver_count);auto r=LowLevelAcceptanceReportModel::build({mk(true,true,"a"),mk(true,false,"b")});C(r.waiverCount==1,"waive");P();}
void t3(){T(blocking_true);auto r=LowLevelAcceptanceReportModel::build({mk(true,false,"a")});C(r.blocking,"blocking");P();}
void t4(){T(blocking_false_when_waived);auto r=LowLevelAcceptanceReportModel::build({mk(true,true,"a")});C(!r.blocking,"blocking");P();}
void t5(){T(json_shape);auto j=LowLevelAcceptanceReportModel::toJson(LowLevelAcceptanceReportModel::build({mk(true,false,"a")}));C(j.contains("total")&&j.contains("entries"),"shape");P();}
void t6(){T(machine_readable);auto j=LowLevelAcceptanceReportModel::toJson(LowLevelAcceptanceReportModel::build({mk(true,false,"a")}));C(j.is_object(),"obj");P();}
void t7(){T(entries_size);auto r=LowLevelAcceptanceReportModel::build({mk(true,false,"a"),mk(false,false,"b")});C(r.entries.size()==2,"size");P();}
void t8(){T(deterministic);auto a=LowLevelAcceptanceReportModel::toJson(LowLevelAcceptanceReportModel::build({mk(true,false,"a"),mk(false,false,"b")})).dump();auto b=LowLevelAcceptanceReportModel::toJson(LowLevelAcceptanceReportModel::build({mk(true,false,"a"),mk(false,false,"b")})).dump();C(a==b,"det");P();}
int main(){std::cout<<"Step 807: LowLevelAcceptanceReport\n";t1();t2();t3();t4();t5();t6();t7();t8();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
