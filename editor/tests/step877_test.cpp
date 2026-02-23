// Step 877: Cost telemetry integration in metrics stack (8 tests)
#include "graduation/CostTelemetryIntegration.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){T(make_record);
    auto r=CostTelemetryIntegration::make("py->cpp","balanced",0.5f,0.4f);
    C(r.pairId=="py->cpp","pairId");P();}
void t2(){T(cost_error_computed);
    auto r=CostTelemetryIntegration::make("py->cpp","balanced",0.5f,0.4f);
    C(std::abs(r.costError-0.1f)<0.001f,"error");P();}
void t3(){T(over_budget_flag);
    auto r=CostTelemetryIntegration::make("py->cpp","balanced",1.2f,0.4f);
    C(r.overBudget,"over");P();}
void t4(){T(not_over_budget);
    auto r=CostTelemetryIntegration::make("py->cpp","balanced",0.8f,0.5f);
    C(!r.overBudget,"not over");P();}
void t5(){T(record_and_count);
    CostTelemetryIntegration cti;
    cti.record(CostTelemetryIntegration::make("py->cpp","balanced",0.5f,0.4f));
    cti.record(CostTelemetryIntegration::make("rust->go","cheap",1.5f,0.3f));
    C(cti.overBudgetCount()==1,"count");P();}
void t6(){T(average_cost_error);
    CostTelemetryIntegration cti;
    cti.record(CostTelemetryIntegration::make("py->cpp","balanced",0.5f,0.3f));
    cti.record(CostTelemetryIntegration::make("py->cpp","balanced",0.7f,0.5f));
    C(std::abs(cti.averageCostError()-0.2f)<0.001f,"avg");P();}
void t7(){T(empty_avg_zero);
    CostTelemetryIntegration cti;
    C(cti.averageCostError()==0.0f,"zero");P();}
void t8(){T(to_json_has_cost_error);
    auto r=CostTelemetryIntegration::make("py->cpp","balanced",0.5f,0.4f);
    auto j=CostTelemetryIntegration::toJson(r);
    C(j.contains("cost_error"),"json");P();}

int main(){
    std::cout<<"Step 877: Cost telemetry integration\n";
    t1();t2();t3();t4();t5();t6();t7();t8();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
