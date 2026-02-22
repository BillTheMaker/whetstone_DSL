// Step 842: PairRegressionDetector tests (10 tests)
#include "graduation/PairRegressionDetector.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(no_events_no_regression);
    auto r=PairRegressionDetector::detect("py->cpp",{});
    C(!r.hasRegression,"no regression");P();}

void t2(){T(single_low_event);
    RegressionEvent e{"py->cpp","F1",RegressionSeverity::Low,"desc","now"};
    auto r=PairRegressionDetector::detect("py->cpp",{e});
    C(r.hasRegression,"has regression");P();}

void t3(){T(critical_event);
    RegressionEvent e{"py->cpp","F1",RegressionSeverity::Critical,"desc","now"};
    auto r=PairRegressionDetector::detect("py->cpp",{e});
    C(r.worstSeverity==RegressionSeverity::Critical,"critical");P();}

void t4(){T(worst_severity_tracked);
    std::vector<RegressionEvent> events={
        {"py->cpp","F1",RegressionSeverity::Low,"d","now"},
        {"py->cpp","F2",RegressionSeverity::High,"d","now"}};
    auto r=PairRegressionDetector::detect("py->cpp",events);
    C(r.worstSeverity==RegressionSeverity::High,"worst=High");P();}

void t5(){T(hasRegression_true_with_events);
    RegressionEvent e{"py->cpp","F1",RegressionSeverity::Medium,"desc","now"};
    auto r=PairRegressionDetector::detect("py->cpp",{e});
    C(r.hasRegression,"hasRegression");P();}

void t6(){T(multiple_events);
    std::vector<RegressionEvent> events={
        {"py->cpp","F1",RegressionSeverity::Low,"d","now"},
        {"py->cpp","F2",RegressionSeverity::Low,"d","now"}};
    auto r=PairRegressionDetector::detect("py->cpp",events);
    C(r.events.size()==2,"2 events");P();}

void t7(){T(empty_pairId);
    auto r=PairRegressionDetector::detect("",{});
    C(r.pairId.empty(),"empty pairId");P();}

void t8(){T(events_stored);
    RegressionEvent e{"py->cpp","F1",RegressionSeverity::Low,"desc","now"};
    auto r=PairRegressionDetector::detect("py->cpp",{e});
    C(r.events[0].featureId=="F1","featureId");P();}

void t9(){T(severity_levels);
    C(RegressionSeverity::None<RegressionSeverity::Critical,"severity ordering");P();}

void t10(){T(toJson_output);
    auto r=PairRegressionDetector::detect("py->cpp",{});
    auto j=PairRegressionDetector::toJson(r);
    C(j.contains("has_regression"),"has_regression");P();}

int main(){
    std::cout<<"Step 842: PairRegressionDetector\n";
    t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
