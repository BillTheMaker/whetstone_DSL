// Step 852: TrendDetector tests (10 tests)
#include "graduation/TrendDetector.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(empty_not_recurring);
    auto r=TrendDetector::detect("T001",{});
    C(!r.recurring,"not recurring");P();}

void t2(){T(single_period_not_recurring);
    TrendDataPoint dp{"2026-W01","T001",3};
    auto r=TrendDetector::detect("T001",{dp});
    C(!r.recurring,"not recurring");P();}

void t3(){T(two_periods_recurring);
    std::vector<TrendDataPoint> pts={{"W01","T001",3},{"W02","T001",4}};
    auto r=TrendDetector::detect("T001",pts);
    C(r.recurring,"recurring");P();}

void t4(){T(increasing_trend);
    std::vector<TrendDataPoint> pts={{"W01","T001",2},{"W02","T001",5}};
    auto r=TrendDetector::detect("T001",pts);
    C(r.increasing,"increasing");P();}

void t5(){T(not_increasing_decreasing);
    std::vector<TrendDataPoint> pts={{"W01","T001",5},{"W02","T001",2}};
    auto r=TrendDetector::detect("T001",pts);
    C(!r.increasing,"not increasing");P();}

void t6(){T(peakCount);
    std::vector<TrendDataPoint> pts={{"W01","T001",3},{"W02","T001",7},{"W03","T001",4}};
    auto r=TrendDetector::detect("T001",pts);
    C(r.peakCount==7,"peak=7");P();}

void t7(){T(periods_count);
    std::vector<TrendDataPoint> pts={{"W01","T001",3},{"W02","T001",4}};
    auto r=TrendDetector::detect("T001",pts);
    C(r.periods==2,"periods=2");P();}

void t8(){T(code_preserved);
    auto r=TrendDetector::detect("T004",{});
    C(r.code=="T004","code");P();}

void t9(){T(toJson_output);
    auto r=TrendDetector::detect("T001",{});
    auto j=TrendDetector::toJson(r);
    C(j.contains("code")&&j.contains("recurring"),"keys");P();}

void t10(){T(filters_by_code);
    std::vector<TrendDataPoint> pts={
        {"W01","T001",3},{"W01","T002",5},{"W02","T001",4}};
    auto r=TrendDetector::detect("T001",pts);
    C(r.periods==2,"2 T001 periods");P();}

int main(){
    std::cout<<"Step 852: TrendDetector\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
