// Step 857: WeeklyQualityReportGenerator tests (8 tests)
#include "graduation/WeeklyQualityReport.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(generate);
    auto r=WeeklyQualityReportGenerator::generate("R1","2026-W08",20,5,8,{"T001","T004"});
    C(r.reportId=="R1","reportId");P();}

void t2(){T(improved_when_resolved_gt_new);
    auto r=WeeklyQualityReportGenerator::generate("R1","W08",20,3,8,{});
    C(r.improved,"improved");P();}

void t3(){T(not_improved_when_new_gt_resolved);
    auto r=WeeklyQualityReportGenerator::generate("R1","W08",20,8,3,{});
    C(!r.improved,"not improved");P();}

void t4(){T(totalFailures_stored);
    auto r=WeeklyQualityReportGenerator::generate("R1","W08",100,10,5,{});
    C(r.totalFailures==100,"totalFailures");P();}

void t5(){T(newFailures_stored);
    auto r=WeeklyQualityReportGenerator::generate("R1","W08",20,7,3,{});
    C(r.newFailures==7,"newFailures");P();}

void t6(){T(resolvedFailures_stored);
    auto r=WeeklyQualityReportGenerator::generate("R1","W08",20,3,10,{});
    C(r.resolvedFailures==10,"resolvedFailures");P();}

void t7(){T(topCodes_stored);
    auto r=WeeklyQualityReportGenerator::generate("R1","W08",20,3,5,{"T001","T002"});
    C(r.topCodes.size()==2,"topCodes");P();}

void t8(){T(toJson_output);
    auto r=WeeklyQualityReportGenerator::generate("R1","W08",20,3,5,{"T001"});
    auto j=WeeklyQualityReportGenerator::toJson(r);
    C(j.contains("report_id")&&j.contains("improved"),"keys");P();}

int main(){
    std::cout<<"Step 857: WeeklyQualityReportGenerator\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
