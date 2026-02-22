// Step 719: Differential execution harness core (12 tests)
#include "equiv/DifferentialExecutionHarness.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static std::vector<DifferentialCaseResult> sample(){return {{"b","x","x",false},{"a","y","y",false}};}

void t1(){T(run_non_empty);auto r=DifferentialExecutionHarness::run(sample());C(r.totalCount==2,"count");P();}
void t2(){T(sorted_by_case_id);auto r=DifferentialExecutionHarness::run(sample());C(r.cases[0].caseId=="a","sort");P();}
void t3(){T(equivalent_true_when_match);auto r=DifferentialExecutionHarness::run(sample());C(r.cases[0].equivalent&&r.cases[1].equivalent,"eq");P();}
void t4(){T(equivalent_count);auto r=DifferentialExecutionHarness::run(sample());C(r.equivalentCount==2,"eqc");P();}
void t5(){T(confidence_one_for_all_eq);auto r=DifferentialExecutionHarness::run(sample());C(r.confidence==1.0,"conf");P();}
void t6(){T(mismatch_detected);auto v=sample();v[0].targetOutput="z";auto r=DifferentialExecutionHarness::run(v);C(r.equivalentCount==1,"mm");P();}
void t7(){T(empty_input_supported);auto r=DifferentialExecutionHarness::run({});C(r.totalCount==0&&r.confidence==0.0,"empty");P();}
void t8(){T(json_shape);auto j=DifferentialExecutionHarness::toJson(DifferentialExecutionHarness::run(sample()));C(j.contains("cases")&&j.contains("equivalent_count"),"shape");P();}
void t9(){T(json_cases_array);auto j=DifferentialExecutionHarness::toJson(DifferentialExecutionHarness::run(sample()));C(j["cases"].is_array(),"arr");P();}
void t10(){T(json_case_fields);auto j=DifferentialExecutionHarness::toJson(DifferentialExecutionHarness::run(sample()));C(j["cases"][0].contains("case_id")&&j["cases"][0].contains("equivalent"),"fields");P();}
void t11(){T(machine_readable);auto j=DifferentialExecutionHarness::toJson(DifferentialExecutionHarness::run(sample()));C(j.is_object(),"obj");P();}
void t12(){T(deterministic);auto a=DifferentialExecutionHarness::toJson(DifferentialExecutionHarness::run(sample())).dump();auto b=DifferentialExecutionHarness::toJson(DifferentialExecutionHarness::run(sample())).dump();C(a==b,"det");P();}

int main(){std::cout<<"Step 719: DifferentialExecutionHarness\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
